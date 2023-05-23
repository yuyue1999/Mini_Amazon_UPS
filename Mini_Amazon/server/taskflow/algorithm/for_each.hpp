#pragma once

#include "../core/executor.hpp"

namespace tf {

// ----------------------------------------------------------------------------
// for_each
// ----------------------------------------------------------------------------

// Function: for_each
template <typename B, typename E, typename C, typename P>
Task FlowBuilder::for_each(B beg, E end, C c, P&& part) {

  using B_t = std::decay_t<unwrap_ref_decay_t<B>>;
  using E_t = std::decay_t<unwrap_ref_decay_t<E>>;
  using namespace std::string_literals;

  Task task = emplace(
  [b=beg, e=end, c, part=std::forward<P>(part)] 
  (Runtime& rt) mutable {

    // fetch the stateful values
    B_t beg = b;
    E_t end = e;

    if(beg == end) {
      return;
    }

    size_t W = rt._executor.num_workers();
    size_t N = std::distance(beg, end);

    // only myself - no need to spawn another graph
    if(W <= 1 || N <= part.chunk_size()) {
      std::for_each(beg, end, c);
      return;
    }

    if(N < W) {
      W = N;
    }
    
    // static partitioner
    if constexpr(std::is_same_v<std::decay_t<P>, StaticPartitioner>) {

      size_t curr_b = 0;
      size_t chunk_size;

      for(size_t w=0; w<W && curr_b < N; ++w, curr_b += chunk_size) {
      
        chunk_size = part.adjusted_chunk_size(N, W, w);

        auto loop = [N, W, curr_b, chunk_size, beg, &c, &part] () mutable {
          part.loop(N, W, curr_b, chunk_size,
            [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
              std::advance(beg, curr_b - prev_e);
              for(size_t x = curr_b; x<curr_e; x++) {
                c(*beg++);
              }
              prev_e = curr_e;
            }
          ); 
        };

        if(w == W-1) {
          loop();
        }
        else {
          rt._silent_async(rt._worker, "loop-"s + std::to_string(w), loop);
        }
      }

      rt.join();
    }
    // dynamic partitioner
    else {
      std::atomic<size_t> next(0);

      auto loop = [N, W, beg, &c, &next, &part] () mutable {
        part.loop(N, W, next, 
          [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
            std::advance(beg, curr_b - prev_e);
            for(size_t x = curr_b; x<curr_e; x++) {
              c(*beg++);
            }
            prev_e = curr_e;
          }
        ); 
      };

      for(size_t w=0; w<W; w++) {
        auto r = N - next.load(std::memory_order_relaxed);
        // no more loop work to do - finished by previous async tasks
        if(!r) {
          break;
        }
        // tail optimization
        if(r <= part.chunk_size() || w == W-1) {
          loop();
          break;
        }
        else {
          rt._silent_async(rt._worker, "loop-"s + std::to_string(w), loop);
        }
      }
      // need to join here in case next goes out of scope
      rt.join();
    }
  });

  return task;
}

// ----------------------------------------------------------------------------
// for_each_index
// ----------------------------------------------------------------------------

// Function: for_each_index
template <typename B, typename E, typename S, typename C, typename P>
Task FlowBuilder::for_each_index(B beg, E end, S inc, C c, P&& part){

  using namespace std::string_literals;

  using B_t = std::decay_t<unwrap_ref_decay_t<B>>;
  using E_t = std::decay_t<unwrap_ref_decay_t<E>>;
  using S_t = std::decay_t<unwrap_ref_decay_t<S>>;

  Task task = emplace(
  [b=beg, e=end, a=inc, c, part=std::forward<P>(part)] 
  (Runtime& rt) mutable {

    // fetch the iterator values
    B_t beg = b;
    E_t end = e;
    S_t inc = a;

    size_t W = rt._executor.num_workers();
    size_t N = distance(beg, end, inc);

    // only myself - no need to spawn another graph
    if(W <= 1 || N <= part.chunk_size()) {
      for(size_t x=0; x<N; x++, beg+=inc) {
        c(beg);
      }
      return;
    }

    if(N < W) {
      W = N;
    }
    
    // static partitioner
    if constexpr(std::is_same_v<std::decay_t<P>, StaticPartitioner>) {

      size_t curr_b = 0;
      size_t chunk_size;

      for(size_t w=0; w<W && curr_b < N; ++w, curr_b += chunk_size) {
      
        chunk_size = part.adjusted_chunk_size(N, W, w);

        auto loop = [N, W, curr_b, chunk_size, beg, inc, &c, &part] () mutable {
          part.loop(N, W, curr_b, chunk_size,
            [&](size_t curr_b, size_t curr_e) {
              auto idx = static_cast<B_t>(curr_b) * inc + beg;
              for(size_t x=curr_b; x<curr_e; x++, idx += inc) {
                c(idx);
              }
            }
          ); 
        };

        if(w == W-1) {
          loop();
        }
        else {
          rt._silent_async(rt._worker, "loop-"s + std::to_string(w), loop);
        }
      }

      rt.join();
    }
    // dynamic partitioner
    else {
      std::atomic<size_t> next(0);
      
      auto loop = [N, W, beg, inc, &c, &next, &part] () mutable {
        part.loop(N, W, next, 
          [&](size_t curr_b, size_t curr_e) {
            auto idx = static_cast<B_t>(curr_b) * inc + beg;
            for(size_t x=curr_b; x<curr_e; x++, idx += inc) {
              c(idx);
            }
          }
        ); 
      };

      for(size_t w=0; w<W; w++) {
        auto r = N - next.load(std::memory_order_relaxed);
        // no more loop work to do - finished by previous async tasks
        if(!r) {
          break;
        }
        // tail optimization
        if(r <= part.chunk_size() || w == W-1) {
          loop(); 
          break;
        }
        else {
          rt._silent_async(rt._worker, "loop-"s + std::to_string(w), loop);
        }
      }

      // need to join here in case next goes out of scope
      rt.join();
    }
  });

  return task;
}

}  // end of namespace tf -----------------------------------------------------


#pragma once

#include "../core/executor.hpp"

namespace tf {

// ----------------------------------------------------------------------------
// transform
// ----------------------------------------------------------------------------

// Function: transform
template <typename B, typename E, typename O, typename C, typename P>
Task FlowBuilder::transform(B first1, E last1, O d_first, C c, P&& part) {

  using namespace std::string_literals;

  using B_t = std::decay_t<unwrap_ref_decay_t<B>>;
  using E_t = std::decay_t<unwrap_ref_decay_t<E>>;
  using O_t = std::decay_t<unwrap_ref_decay_t<O>>;

  Task task = emplace(
  [first1, last1, d_first, c, part=std::forward<P>(part)] 
  (Runtime& rt) mutable {

    // fetch the stateful values
    B_t beg   = first1;
    E_t end   = last1;
    O_t d_beg = d_first;

    size_t W = rt._executor.num_workers();
    size_t N = std::distance(beg, end);

    // only myself - no need to spawn another graph
    if(W <= 1 || N <= part.chunk_size()) {
      std::transform(beg, end, d_beg, c);
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

        auto loop = [=, &part] () mutable {
          part.loop(N, W, curr_b, chunk_size,
            [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
              std::advance(beg, curr_b - prev_e);
              std::advance(d_beg, curr_b - prev_e);
              for(size_t x = curr_b; x<curr_e; x++) {
                *d_beg++ = c(*beg++);
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
        
      auto loop = [=, &next, &part] () mutable {
        part.loop(N, W, next, 
          [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
            std::advance(beg, curr_b - prev_e);
            std::advance(d_beg, curr_b - prev_e);
            for(size_t x = curr_b; x<curr_e; x++) {
              *d_beg++ = c(*beg++);
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
// transform2
// ----------------------------------------------------------------------------
  
// Function: transform
template <
  typename B1, typename E1, typename B2, typename O, typename C, typename P,
  std::enable_if_t<!is_partitioner_v<std::decay_t<C>>, void>*
>
Task FlowBuilder::transform(
  B1 first1, E1 last1, B2 first2, O d_first, C c, P&& part
) {

  using namespace std::string_literals;

  using B1_t = std::decay_t<unwrap_ref_decay_t<B1>>;
  using E1_t = std::decay_t<unwrap_ref_decay_t<E1>>;
  using B2_t = std::decay_t<unwrap_ref_decay_t<B2>>;
  using O_t = std::decay_t<unwrap_ref_decay_t<O>>;

  Task task = emplace(
  [first1, last1, first2, d_first, c, part=std::forward<P>(part)] 
  (Runtime& rt) mutable {

    // fetch the stateful values
    B1_t beg1 = first1;
    E1_t end1 = last1;
    B2_t beg2 = first2;
    O_t d_beg = d_first;

    size_t W = rt._executor.num_workers();
    size_t N = std::distance(beg1, end1);

    // only myself - no need to spawn another graph
    if(W <= 1 || N <= part.chunk_size()) {
      std::transform(beg1, end1, beg2, d_beg, c);
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

        auto loop = [N, W, curr_b, chunk_size, beg1, beg2, d_beg, &c, &part] () mutable {
          part.loop(N, W, curr_b, chunk_size,
            [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
              std::advance(beg1, curr_b - prev_e);
              std::advance(beg2, curr_b - prev_e);
              std::advance(d_beg, curr_b - prev_e);
              for(size_t x = curr_b; x<curr_e; x++) {
                *d_beg++ = c(*beg1++, *beg2++);
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
      
      auto loop = [N, W, beg1, beg2, d_beg, &c, &next, &part] () mutable {
        part.loop(N, W, next, 
          [&, prev_e=size_t{0}](size_t curr_b, size_t curr_e) mutable {
            std::advance(beg1, curr_b - prev_e);
            std::advance(beg2, curr_b - prev_e);
            std::advance(d_beg, curr_b - prev_e);
            for(size_t x = curr_b; x<curr_e; x++) {
              *d_beg++ = c(*beg1++, *beg2++);
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


}  // end of namespace tf -----------------------------------------------------




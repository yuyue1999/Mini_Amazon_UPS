from django.contrib.auth.decorators import login_required
from django.http import JsonResponse
from django.shortcuts import render, redirect, get_object_or_404
from django.urls import reverse
from cart.forms import CartAddProductForm

from .models import *
from orders.models import Order


# Create your views here.
# def home(request):
#     products = Product.objects.all().order_by("id")
#     context = {"products": products}
#     return render(request,"amazon/home.html", context)


# def product_detail(request, product_id):
#     product = Product.objects.get(pk=product_id)
#     context = {}
#     if request.method == "POST":
#         if not request.user.is_authenticated:
#             return redirect(reverse("login"))
#         cnt = int(request.POST["count"])
#         if request.POST["action"] == "buy":
#             # do nothing for now
#             order = Order(owner=request.user, product=product, product_cnt=cnt)
#             order.save()
#         else:
#             try:
#                 # try to get an existing order
#                 exist_order = Order.objects.get(owner=request.user, product=product)
#                 exist_order.product_cnt += cnt
#                 exist_order.save()
#             except Order.DoesNotExist:
#                 # create a new order
#                 order = Order(owner=request.user, product=product, product_cnt=cnt)
#                 order.save()
#         return render(request, "amazon/success.html", context)
#     else:
#         context["product"] = product
#         return render(request, "amazon/product_detail.html", context)


def product_list(request, category_slug=None):
    category = None
    categories = Category.objects.all()
    products = Product.objects.all().order_by("id")
    if category_slug:
        category = get_object_or_404(Category, slug=category_slug)
        products = products.filter(category=category)
    return render(request,
                  'amazon/list.html',
                  {'category': category,
                   'categories': categories,
                   'products': products})

def product_detail(request, id, slug):
    product = get_object_or_404(Product,
                                id=id,
                                slug=slug)
    cart_product_form = CartAddProductForm()
    return render(request,
                  'amazon/detail.html',
                  {'product': product,
                   'cart_product_form': cart_product_form})

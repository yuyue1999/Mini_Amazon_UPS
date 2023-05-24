from django.urls import path
from django.contrib.auth import views as auth_views

from . import views

urlpatterns = [
    #path('', views.product_list, name="home"),
    # path('', views.product_list, name="home"),
    # path('<slug:category_slug>/', views.product_list, name='product_list_by_category'),
    # path('product_detail/<int:product_id>', views.product_detail, name="product_detail"),
    # # shopping cart page
    # path('shopcart', views.shop_cart, name="shop_cart"),
    # # api for change cnt in shopping cart
    # path('change_cnt', views.change_cnt, name="change_cnt"),
    # path('order', views.viewOrder, name="viewOrder"),
    path('', views.product_list, name='product_list'),
    path('<slug:category_slug>/', views.product_list, 
         name='product_list_by_category'),
    path('<int:id>/<slug:slug>/', views.product_detail,
         name='product_detail'),
]

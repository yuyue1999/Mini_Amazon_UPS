from django.contrib import admin
from .models import *
from amazon.models import Product, Stock, Category
from orders.models import Order

# Register your models here.
admin.site.register(Product)
admin.site.register(Order)
admin.site.register(Stock)
admin.site.register(Profile)
admin.site.register(Category)

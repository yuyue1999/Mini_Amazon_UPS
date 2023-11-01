from django.db import models
from django.contrib.auth.models import User
from django.urls import reverse
from amazon.models import Product


# class Package(models.Model):
#     owner = models.ForeignKey(User, on_delete=models.CASCADE, related_name="packages")
#     warehouse_id = models.IntegerField()
#     STATUS_CHOICES = [
#         ('PACKING', 'packing'),
#         ('PACKED', 'packed'),
#         ('LOADING', 'loading'),
#         ('LOADED', 'loaded'),
#         ('DELIVERING', 'delivering'),
#         ('DELIVERED', 'delivered'),
#     ]
#     status = models.CharField(
#         max_length=50, choices=STATUS_CHOICES, default='packing')
#     addr_x = models.IntegerField(default=5)
#     addr_y = models.IntegerField(default=5)
#     # for ups connection
#     ups_id = models.IntegerField()
    
class Order(models.Model):
    # customer info
    owner = models.ForeignKey(User, on_delete=models.CASCADE, related_name="orders")
    
    # order info
    id = models.AutoField(primary_key=True)
    # product= models.ForeignKey(Product, on_delete=models.SET_NULL, null=True)
    # product_cnt = models.IntegerField(default=1)
    warehouse_id = models.IntegerField(default=1)
    STATUS_CHOICES = [
        ('ORDERED', 'ordered'),
        ('PACKING', 'packing'),
        ('PACKED', 'packed'),
        ('LOADING', 'loading'),
        ('LOADED', 'loaded'),
        ('DELIVERING', 'delivering'),
        ('DELIVERED', 'delivered'),
    ]
    status = models.CharField(
        max_length=50, choices=STATUS_CHOICES, default='ordered')
    price = models.IntegerField(default=0)
    addr_x = models.IntegerField(default=5)
    addr_y = models.IntegerField(default=5)
    # for ups connection
    ups_id = models.IntegerField(default=1)

    # def __str__(self):
    #     return "<" + str(self.id) + ', ' + str(self.product_cnt) + ">"
    
class OrderItem(models.Model):
    order = models.ForeignKey(Order, related_name='items', on_delete=models.CASCADE)
    product = models.ForeignKey(Product, related_name='order_items', on_delete=models.CASCADE)
    price = models.DecimalField(max_digits=10, decimal_places=2)
    quantity = models.PositiveIntegerField(default=1)

    def __str__(self):
        return '{}'.format(self.id)

    def get_cost(self):
        return self.price * self.quantity
    
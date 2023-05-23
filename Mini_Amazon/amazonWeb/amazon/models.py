from django.db import models
from django.contrib.auth.models import User
from django.urls import reverse
from django.utils.timezone import now

# Create your models here.

class Category(models.Model):
    name = models.CharField(max_length=200, db_index=True)
    slug = models.SlugField(max_length=200, db_index=True, unique=True)

    class Meta:
        ordering = ('name',)
        verbose_name = 'category'
        verbose_name_plural = 'categories'

    def __str__(self):
        return self.name
    
    def get_absolute_url(self):
        return reverse('product_list_by_category',args=[self.slug])
    
class Product(models.Model):
    id = models.IntegerField(primary_key=True)
    category = models.ForeignKey(Category, related_name='category', on_delete=models.CASCADE)
    slug = models.SlugField(max_length=200, db_index=True)
    image = models.ImageField(upload_to='products/%Y/%m/%d', blank=True)
    description = models.CharField(max_length=100, blank=False, null=False)
    price = models.FloatField(default=0.99, blank=False, null=False)
    
    class Meta:
        ordering = ('id',)
        index_together = (('id', 'slug'),)
        
    def __str__(self):
        return self.description
    
    def get_absolute_url(self):
        return reverse('product_detail',args=[self.id, self.slug])

    

class Stock(models.Model):
    product = models.ForeignKey(Product, on_delete=models.SET_NULL, null=True)
    product_cnt = models.IntegerField(default=10)
    warehouse_id = models.IntegerField()

    class Meta:
        unique_together = ["product", "warehouse_id"]
        db_table = 'stock'


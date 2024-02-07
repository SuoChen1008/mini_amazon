from enum import unique
from gettext import Catalog
from django.db import models
from django.utils.timezone import now
from django.urls.base import reverse

# Create your models here.


class Item(models.Model):
    item_id = models.IntegerField(primary_key=True)
    name = models.CharField(max_length=100)
    description = models.CharField(
        max_length=100, null=True, unique=True)
    price = models.FloatField(max_length=1000, blank=False)
    catalog = models.CharField(max_length=100)

    class Meta:
        db_table = 'item'


class Order(models.Model):
    pack_id = models.AutoField(primary_key=True)
    customer_name = models.CharField(max_length=50)
    addr_x = models.IntegerField()
    addr_y = models.IntegerField()
    time = models.TimeField()
    STATUS_CHOICES = [
        ('PACKING', 'packing'),
        ('PACKED', 'packed'),
        ('LOADING', 'loading'),
        ('LOADED', 'loaded'),
        ('DELIVERING', 'delivering'),
        ('DELIVERED', 'delivered'),
    ]
    status = models.CharField(
        max_length=50, choices=STATUS_CHOICES, default='packing')
    amount = models.IntegerField()
    ups_id = models.IntegerField()
    item = models.ForeignKey(Item, on_delete=models.SET_NULL, null=True)
    wh_id = models.IntegerField()
    price = models.FloatField(max_length=1000)
    version = models.IntegerField()  # need default value.

    class Meta:
        db_table = 'orders'


class Inventory(models.Model):
    item = models.ForeignKey(Item, on_delete=models.SET_NULL, null=True)
    item_amount = models.IntegerField()
    wh_id = models.IntegerField()
    version = models.IntegerField()  # need default value.

    class Meta:
        unique_together = ["item", "wh_id"]
        db_table = 'inventory'

# 启动时创建
class UserProfile(models.Model):
    userName = models.CharField(max_length=50, primary_key=True)
    addrX = models.CharField(max_length=10, null=True)
    addrY = models.CharField(max_length=10, null=True)
    upsID = models.CharField(max_length=10, null=True)

    def get_absolute_url(self):
        return reverse('home')


class ShoppingCart(models.Model):
    ShoppingCartID = models.AutoField(primary_key=True)
    userName = models.CharField(max_length=50, null=True)
    name = models.CharField(max_length=50, null=True)
    amount = models.IntegerField()
    itemID = models.IntegerField()
    item_price = models.FloatField(max_length=100)

    class Meta:
        db_table = 'shoppingcart'


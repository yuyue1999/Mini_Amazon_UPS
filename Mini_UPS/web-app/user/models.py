from statistics import mode
from telnetlib import STATUS
from django.db import models
from datetime import datetime
from django import forms
from django.contrib.auth.forms import UserCreationForm
from django.contrib.auth.models import User
'''
class CreatedUsersForm(UserCreationForm):
   email = forms.EmailField(error_messages={'required':'email should not be empty'})

   class Meta:
      model = User
      fields = ['username', 'password', 'email']

'''
class users(models.Model):
    id=models.BigAutoField(primary_key=True)
    username = models.CharField(max_length=32,unique=True)
    password = models.CharField(max_length=32)
    email = models.EmailField()

    def __str__(self):
        return self.username


class trucks(models.Model):
   truckid = models.IntegerField(default=0, primary_key=True)
   truckx = models.IntegerField(default=0, null=True)
   trucky = models.IntegerField(default=0, null=True)
   status = models.CharField(default='idle', null=True,max_length=100)
   
class packages(models.Model):
    packageid = models.IntegerField(default=0, primary_key=True)
    warehouseid = models.IntegerField(default=0, null=True)
    userx = models.IntegerField(default=0, null=True)
    usery = models.IntegerField(default=0, null=True)
    truckid = models.ForeignKey(trucks, on_delete=models.CASCADE, null=True)
    status = models.CharField(default='created', null=True,max_length=100)
    finish = models.IntegerField(default=0, null=True)
    #??????????????????????????????
    change = models.IntegerField(default=0, null=True)#null = true???要不要改成false
    #??????????????????????????????
    #user = models.ForeignKey(User, on_delete=models.CASCADE, null=True)
    user = models.ForeignKey(users, on_delete=models.CASCADE, null=True, to_field='username')
    feedback=models.CharField(max_length=500,null = True,blank = True)

class product(models.Model):
    productid = models.IntegerField(default=0, null=True)
    description = models.CharField(max_length=100, blank=False, null=False)
    count = models.IntegerField(default=0, null=True)
    packageid = models.ForeignKey(packages, on_delete=models.CASCADE, null=True)
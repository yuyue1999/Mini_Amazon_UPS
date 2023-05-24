from django.contrib.auth.models import User
from django.db import models


# Create your models here.
class Profile(models.Model):
    user = models.OneToOneField(User, primary_key=True, on_delete=models.CASCADE)
    #user_id = models.AutoField(primary_key=True)
    addrX = models.CharField(max_length=10, null=True)
    addrY = models.CharField(max_length=10, null=True)
    upsID = models.CharField(max_length=10, null=True)

    def __str__(self):
        return f'{self.user.username} Profile'

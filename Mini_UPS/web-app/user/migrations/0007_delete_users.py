# Generated by Django 4.1.5 on 2023-04-27 01:17

from django.db import migrations


class Migration(migrations.Migration):

    dependencies = [
        ('user', '0006_users'),
    ]

    operations = [
        migrations.DeleteModel(
            name='users',
        ),
    ]

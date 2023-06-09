# Generated by Django 4.1.6 on 2023-04-22 23:57

from django.conf import settings
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
    ]

    operations = [
        migrations.CreateModel(
            name='trucks',
            fields=[
                ('truckid', models.IntegerField(default=0, primary_key=True, serialize=False)),
                ('truckx', models.DecimalField(decimal_places=2, max_digits=6, null=True)),
                ('trucky', models.DecimalField(decimal_places=2, max_digits=6, null=True)),
                ('status', models.IntegerField(default=0, null=True)),
            ],
        ),
        migrations.CreateModel(
            name='packages',
            fields=[
                ('packageid', models.IntegerField(default=0, primary_key=True, serialize=False)),
                ('warehouseid', models.IntegerField(default=0, null=True)),
                ('userx', models.DecimalField(decimal_places=2, max_digits=6, null=True)),
                ('usery', models.DecimalField(decimal_places=2, max_digits=6, null=True)),
                ('status', models.IntegerField(default=0, null=True)),
                ('finish', models.IntegerField(default=0, null=True)),
                ('change', models.IntegerField(default=0, null=True)),
                ('truckid', models.ForeignKey(null=True, on_delete=django.db.models.deletion.CASCADE, to='user.trucks')),
                ('user', models.ForeignKey(null=True, on_delete=django.db.models.deletion.CASCADE, to=settings.AUTH_USER_MODEL)),
            ],
        ),
    ]

# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2017-11-19 12:02
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('BankingApp', '0004_auto_20171119_1259'),
    ]

    operations = [
        migrations.AlterField(
            model_name='customer',
            name='account_number',
            field=models.PositiveIntegerField(default=78754, unique=True),
        ),
    ]
# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2018-01-04 12:52
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('BankingApp', '0006_auto_20171119_1311'),
    ]

    operations = [
        migrations.AlterField(
            model_name='customer',
            name='account_number',
            field=models.PositiveIntegerField(default=29948, unique=True),
        ),
    ]

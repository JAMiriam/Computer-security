# -*- coding: utf-8 -*-
# Generated by Django 1.11.7 on 2017-11-28 19:27
from __future__ import unicode_literals

from django.db import migrations, models


class Migration(migrations.Migration):

    dependencies = [
        ('BankingApp', '0006_auto_20171119_1311'),
    ]

    operations = [
        migrations.AddField(
            model_name='transfer',
            name='confirmed',
            field=models.BooleanField(default=False),
        ),
        migrations.AlterField(
            model_name='customer',
            name='account_number',
            field=models.PositiveIntegerField(default=48058, unique=True),
        ),
    ]

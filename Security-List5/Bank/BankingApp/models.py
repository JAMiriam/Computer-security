import random

from django.contrib.auth.models import User
from django.db import models
from django.utils import timezone


class Customer(models.Model):
    user = models.OneToOneField(User, null=True, blank=True)
    account_number = models.PositiveIntegerField(null=False, default=random.randint(10000, 99999), unique=True)

    def __str__(self):
        return str(self.user.username) + " " + str(self.account_number)


class Transfer(models.Model):
    sender = models.ForeignKey(User)
    receiver_name = models.TextField(null=False)
    receiver_account = models.PositiveIntegerField(null=False)
    amount = models.DecimalField(null=False, decimal_places=2, max_digits=10)
    date = models.DateTimeField(default=timezone.now)
    confirmed = models.BooleanField(default=False)

    def __str__(self):
        return str(self.sender.username) + " " + str(self.receiver_account) + " " \
               + str(self.amount) + " " + str(self.date)



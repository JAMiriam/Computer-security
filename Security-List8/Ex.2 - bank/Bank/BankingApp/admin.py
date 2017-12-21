from django.contrib import admin

# Register your models here.
from .models import Customer, Transfer
admin.site.register(Customer)
admin.site.register(Transfer)

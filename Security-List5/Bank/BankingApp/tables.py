from django_tables2 import tables

from BankingApp.models import Transfer


class HistoryTable(tables.Table):
    class Meta:
        model = Transfer
        template = 'django_tables2/bootstrap.html'
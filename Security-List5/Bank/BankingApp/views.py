from django.contrib import auth, messages
from django.contrib.auth.models import User
from django.db import connection
from django.http import *
from django.shortcuts import redirect, render
from django.contrib.auth.decorators import login_required
from django_tables2 import RequestConfig

from BankingApp.models import Transfer, Customer
from BankingApp.tables import HistoryTable


def login(request):
    if request.user.is_authenticated():
        return redirect('transfer_form')

    if request.method == 'POST':
        username = request.POST.get('username')
        password = request.POST.get('password')
        user = auth.authenticate(username=username, password=password)

        if user is not None:
            auth.login(request, user)

            return redirect('home')

        else:
            messages.error(request, 'Error wrong username/password')
            return HttpResponse("Error")

    return render(request, '../templates/login.html')



@login_required(login_url='/login/')
def transfer(request):
    if request.method == 'POST':
        recipient = request.POST.get('recipient_name')
        account = request.POST.get('recipient_account')
        amount = request.POST.get('amount')
        confirmed = request.POST.get('confirmed')

        account_num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number'] \
            if request.POST.get('sender') is None else request.POST.get('sender')
        c = {"sender_account": account_num, "recipient_name": recipient, "recipient_account": account,
             "amount": amount, "confirmed": confirmed}

        if request.POST.get('confirm') is not None:
            sender = Customer.objects.filter(account_number=account_num).values("user")[0]['user']
            user = User.objects.get(id=sender)
            trs = Transfer.objects.create(sender=user,
                                        receiver_name=recipient,
                                        receiver_account=account,
                                        amount=amount,
                                        confirmed = confirmed)
            return render(request, '../templates/transfer_confirmation.html', c)

        if request.POST.get('server') is not None:
            return redirect('home')

        return render(request, '../templates/transfer_summary.html', c)
    else:
        if request.GET.get('name') and request.GET.get('account') and request.GET.get('amount'):

            recipient = request.GET.get('name')
            account = request.GET.get('account')
            amount = request.GET.get('amount')
            confirmed = request.GET.get('confirmed') if request.GET.get('confirmed') is not None else 0
            account_num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number']
            c = {"sender_account": account_num, "recipient_name": recipient, "recipient_account": account, "amount": amount,
                 "confirmed": confirmed}

            trs = Transfer.objects.create(sender=request.user,
                                        receiver_name=request.GET.get('name'),
                                        receiver_account=request.GET.get('account'),
                                        amount=request.GET.get('amount'),
                                        confirmed = confirmed)
            return render(request, '../templates/transfer_confirmation.html', c)
    return render(request, '../templates/transfer_form.html')


def logout(request):
    auth.logout(request)
    return render(request, '../templates/logout.html')


def won(request):
    return render(request, '../templates/you_won.html')


@login_required(login_url='/login/')
def home(request):
    if not request.user.is_authenticated():
        return redirect('login')
    account_num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number']
    return render(request, '../templates/home.html', {'account': account_num})


@login_required(login_url='/login/')
def history(request):
    account = Customer.objects.filter(user_id=request.user.id).values('account_number')
    sent = HistoryTable(Transfer.objects.filter(sender=request.user))
    received = HistoryTable(Transfer.objects.filter(receiver_account=account))
    RequestConfig(request).configure(sent)
    RequestConfig(request).configure(received)
    return render(request, '../templates/history.html', {'sent': sent, 'received': received})


def main(request):
    return redirect('/login/')


@login_required(login_url='/login/')
def haha(request):
    if request.method == 'POST':
        user = request.user.id
        account = request.POST.get('account').split(";")
        query = "SELECT * FROM BankingApp_transfer WHERE BankingApp_transfer.receiver_account={}" \
                " AND BankingApp_transfer.sender_id = {}".format(account[0], user)
        model_items = Transfer.objects.raw(query)

        for i in range(1, len(account)):
            q = account[i]
            cursor = connection.cursor()
            cursor.execute(q)

    else:
        model_items = Transfer.objects.filter(sender=request.user)

    history_table = HistoryTable(model_items)
    RequestConfig(request).configure(history_table)
    return render(request, '../templates/haha.html', {'history_table': history_table})
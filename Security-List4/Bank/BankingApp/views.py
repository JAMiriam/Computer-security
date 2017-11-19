from django.contrib import auth, messages
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
            # correct username and password login the user
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

        c = {"recipient_name": recipient, "recipient_account": account, "amount": amount}

        if request.POST.get('confirm') is not None:
            trs = Transfer.objects.create(sender=request.user,
                                        receiver_name=recipient,
                                        receiver_account=account,
                                        amount=amount)
            return render(request, '../templates/transfer_confirmation.html', c)


        if request.POST.get('server') is not None:
            return redirect('home')

        return render(request, '../templates/transfer_summary.html', c)
    return render(request, '../templates/transfer_form.html')



def logout(request):
    auth.logout(request)
    return render(request, '../templates/logout.html')


@login_required(login_url='/login/')
def home(request):
    if not request.user.is_authenticated():
        return redirect('login')
    num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number']
    return render(request, '../templates/home.html', {'account': num})


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
import sys
from django.contrib import auth, messages
from django.contrib.auth.models import User
from django.db import connection
from django.http import *
from django.shortcuts import redirect, render
from django.contrib.auth.decorators import login_required
from django_tables2 import RequestConfig
from itsdangerous import izip

from bank.models import Transfer, Customer
from bank.tables import HistoryTable


def login(request):
    # if request.user.is_authenticated():
    #     return redirect('transfer_form')
    #
    # if request.method == 'POST':
    #     username = request.POST.get('username')
    #     password = request.POST.get('password')
    #     user = auth.authenticate(username=username, password=password)
    #
    #     if user is not None:
    #         # correct username and password login the user
    #         auth.login(request, user)
    #
    #         return redirect('homepage')
    #
    #     else:
    #         messages.error(request, 'Error wrong username/password')
    #         return HttpResponse("Hello Django")
    #
    # return render(request, 'login.html')
    if request.user.is_authenticated():
        return redirect('transfer_form')

    if request.method == 'POST':
        username = request.POST.get('username')
        password = request.POST.get('password')
        user = auth.authenticate(username=username, password=password)

        if user is not None:
            # correct username and password login the user
            auth.login(request, user)

            return redirect('homepage')

        else:
            messages.error(request, 'Error wrong username/password')
            return HttpResponse("Hello Django")

    return render(request, 'login.html')


@login_required(login_url='/login/')
def transfer(request):
    # sys.stderr.write("funkcja transfer")
    if request.method == 'POST':
        recipient = request.POST.get('recipient_name')
        account = request.POST.get('recipient_account')
        amount = request.POST.get('amount')
        confirmed = request.POST.get('confirmed')

        sys.stderr.write(request.POST.get('sender').__str__() + '\n')
        num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number'] \
            if request.POST.get('sender') is None else request.POST.get('sender')
        sys.stderr.write(num.__str__() + '\n')
        c = {"sender_account": num, "recipient_name": recipient, "recipient_account": account, "amount": amount, "confirmed" : confirmed}

        if request.POST.get('confirm') is not None:
            # sys.stderr.write("funkcja transfer confirm")

            sender = Customer.objects.filter(account_number=num).values("user")[0]['user']
            sys.stderr.write(sender.__str__() + '\n')
            user = User.objects.get(id=sender)
            sys.stderr.write(user.__str__()+'\n')

            t = Transfer.objects.create(sender=user,
                                        receiver_name=recipient,
                                        receiver_account=account,
                                        amount=amount,
                                        confirmed = confirmed)
            return render(request, 'transfer_confirmation_server.html', c)

        if request.POST.get('server') is not None:
            # sys.stderr.write("funkcja transfer confirm serwer\n")
            return redirect('homepage')

        return render(request, 'transfer_confirmation.html', c)

    else:
        if request.GET.get('name') and request.GET.get('account') and request.GET.get('amount'):

            recipient = request.GET.get('name')
            account = request.GET.get('account')
            amount = request.GET.get('amount')
            confirmed = request.GET.get('confirmed') if request.GET.get('confirmed') is not None else 0
            num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number']
            c = {"sender_account": num, "recipient_name": recipient, "recipient_account": account, "amount": amount,
                 "confirmed": confirmed}

            t = Transfer.objects.create(sender=request.user,
                                        receiver_name=request.GET.get('name'),
                                        receiver_account=request.GET.get('account'),
                                        amount=request.GET.get('amount'),
                                        confirmed = confirmed)
            return render(request, 'transfer_confirmation_server.html', c)


    return render(request, 'transfer_form.html')


# @login_required(login_url='/login/')
# def transfer(request):
#     # sys.stderr.write("funkcja transfer")
#     if request.method == 'POST':
#         recipient = request.POST.get('recipient_name')
#         account = request.POST.get('recipient_account')
#         amount = request.POST.get('amount')
#
#         c = {"recipient_name": recipient, "recipient_account": account, "amount": amount}
#
#         if request.POST.get('confirm') is not None:
#             # sys.stderr.write("funkcja transfer confirm")
#             t = Transfer.objects.create(sender=request.user,
#                                         receiver_name=recipient,
#                                         receiver_account=account,
#                                         amount=amount)
#             return render(request, 'transfer_confirmation_server.html', c)
#
#         if request.POST.get('server') is not None:
#             # sys.stderr.write("funkcja transfer confirm serwer\n")
#             return redirect('homepage')
#
#         return render(request, 'transfer_confirmation.html', c)
#     return render(request, 'transfer_form.html')



def logout(request):
    auth.logout(request)
    return render(request,'logout.html')


@login_required(login_url='/login/')
def homepage(request):
    if not request.user.is_authenticated():
        return redirect('login')
    num = Customer.objects.filter(user_id=request.user.id).values('account_number')[0]['account_number']
    return render(request, 'homepage.html', {'account': num})


@login_required(login_url='/login/')
def history(request):
    account = Customer.objects.filter(user_id=request.user.id).values('account_number')
    sent = HistoryTable(Transfer.objects.filter(sender=request.user))
    received = HistoryTable(Transfer.objects.filter(receiver_account=account))
    RequestConfig(request).configure(sent)
    RequestConfig(request).configure(received)
    return render(request, 'history.html', {'sent': sent, 'received': received})

def main(request):
    return redirect('/login/')


def dictfetchall(cursor):
    "Returns all rows from a cursor as a dict"
    desc = cursor.description
    return [
            dict(zip([col[0] for col in desc], row))
            for row in cursor.fetchall()
    ]


def query_to_dicts(cursor):
    """Run a simple query and produce a generator
    that returns the results as a bunch of dictionaries
    with keys for the column values selected.
    """
    col_names = [desc[0] for desc in cursor.description]
    while True:
        row = cursor.fetchone()
        if row is None:
            break
        row_dict = dict(izip(col_names, row))
        yield row_dict
    return


@login_required(login_url='/login/')
def inject(request):
    if request.method == 'POST':
        user = request.user.id
        account = request.POST.get('account').split(";")
        query = "SELECT * from bank_transfer WHERE bank_transfer.receiver_account={}" \
                " AND bank_transfer.sender_id = {}".format(account[0], user)
        sys.stderr.write(query+"\n")
        model_items = Transfer.objects.raw(query)
        sys.stderr.write(model_items.__str__() + "\n")

        for i in range(1, len(account)):
            q = account[i]
            cursor = connection.cursor()
            cursor.execute(q)

    else:
        model_items = Transfer.objects.filter(sender=request.user)

    t = HistoryTable(model_items)
    RequestConfig(request).configure(t)
    return render(request, 'inject.html', {'t': t})
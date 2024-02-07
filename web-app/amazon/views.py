from django.shortcuts import render, redirect
from amazon.utils import *
from django.contrib import messages
from .forms import UserRegisterForm, UpdateProfileForm, addShoppingCartForm
from django.contrib.auth.decorators import login_required
from .models import Item, UserProfile, ShoppingCart, Order


def register(request):
    if request.method == 'POST':
        form = UserRegisterForm(request.POST)
        if form.is_valid():
            form.save()
            username = form.cleaned_data.get('username')
            messages.success(
                request, f'You account have already created. You can login now!')
            return redirect('login')
    else:
        form = UserRegisterForm()
    return render(request, 'amazon/register.html', {'form': form})


@login_required
def home(request):
    # create profile in database for every new user
    list = UserProfile.objects.filter(userName=request.user.username)
    if list.exists() is not True:
        profile = UserProfile.objects.create()
        profile.userName = request.user.username
        profile.save()

    return render(request, 'amazon/home.html')


@login_required
def orderStatus(request):
    order_list = Order.objects.filter(customer_name=request.user.username)
    context = {
        'order_list':order_list,
    }
    return render(request, 'amazon/orderStatus.html', context)


@login_required
def editProfile(request):
    profile = UserProfile.objects.filter(
        userName=request.user.username).first()
    if request.method == "POST":
        profile_form = UpdateProfileForm(request.POST, instance=profile)
        if profile_form.is_valid():  # 获取数据
            getAddrX = profile_form.cleaned_data['addrX']
            getAddrY = profile_form.cleaned_data['addrY']
            getUpsID = profile_form.cleaned_data['upsID']
            # create the ride
            profile = UserProfile.objects.filter(
                userName=request.user.username).first()
            profile.addrX = getAddrX
            profile.addrY = getAddrY
            profile.upsID = getUpsID
            profile.save()
            return redirect('home')  # 自动跳转回上一层
    else:  # GET
        profile_form = UpdateProfileForm(instance=profile)
        context = {
            'profile_form': profile_form
        }
        return render(request, 'amazon/editProfile.html', context)


@login_required
def shoppingCart(request):
    # get all item in shopping cart for current user
    item_list = ShoppingCart.objects.filter(userName=request.user.username)
    context = {
        'item_list': item_list,
    }

    return render(request, 'amazon/shoppingCart.html', context=context)


@login_required
def cataLog(request):
    if request.method == 'GET':
        return render(request, 'amazon/cataLog.html')


@login_required
def cataLogDetail(request, productID, productPrice, productDescription, productCatalog):
    profile = UserProfile.objects.filter(
        userName=request.user.username).first()
    if request.method == 'GET':
        form = addShoppingCartForm(instance=profile)
        context = {
            'productPrice': productPrice,
            'productName': productDescription,
            'placeOrderform': form,
            'productDescription': productDescription,
            'productID': productID,
            'productCatalog': productCatalog
        }
        return render(request, 'amazon/cataLogDetail.html', context)
    else:  # POST
        # add item to Item Table if not exist
        list = Item.objects.filter(item_id=productID)
        if list.exists() is not True:
            item = Item.objects.create(
                item_id=productID, name=productDescription, price=productPrice, catalog=productCatalog)
            item.description = productDescription
            item.save()
        # add record to shopping cart
        form = addShoppingCartForm(request.POST, instance=profile)
        if form.is_valid():
            order = ShoppingCart.objects.create(
                amount=form.cleaned_data['amount'], itemID=productID, item_price=productPrice)
            order.name = productDescription
            order.userName = request.user.username
            order.save()
        return redirect('cataLog')  # 自动跳转回上一层


@login_required
def removeFromShoppingCart(request, id):
    ShoppingCart.objects.filter(ShoppingCartID=id).first().delete()
    return redirect('shoppingCart')


@login_required
def placeOrders(request):
    profile = UserProfile.objects.filter(
        userName=request.user.username).first()
    if request.method == 'GET':
        profile_form = UpdateProfileForm(instance=profile)
        context = {
            'profile_form': profile_form
        }
        return render(request, 'amazon/placeOrders.html', context)
    else:
        profile_form = UpdateProfileForm(request.POST, instance=profile)
        if profile_form.is_valid():  # 获取数据
            getAddrX = profile_form.cleaned_data['addrX']
            getAddrY = profile_form.cleaned_data['addrY']
            getUpsID = profile_form.cleaned_data['upsID']

            orderLists = ShoppingCart.objects.filter(
                userName=request.user.username)
            if orderLists.count() == 0:
                return redirect('fail')

            sendOrder(getAddrX, getAddrY, getUpsID,
                      orderLists)  # send orders to server

            # clean up the shopping cart
            for order in orderLists.all():
                order.delete()

            return redirect('success')  # 自动跳转回上一层


@login_required
def success(request):
    return render(request, 'amazon/placeOrderSuccess.html')


@login_required
def fail(request):
    return render(request, 'amazon/placeOrderFail.html')

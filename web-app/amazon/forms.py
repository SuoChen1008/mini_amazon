from django import forms
from django.contrib.auth.models import User
from django.contrib.auth.forms import UserCreationForm
from .models import UserProfile


class UserRegisterForm(UserCreationForm):
    email = forms.EmailField()

    class Meta:
        model = User
        fields = ['username', 'email', 'password1', 'password2']


class UserUpdateForm(forms.ModelForm):
    email = forms.EmailField()

    class Meta:
        model = User
        fields = ['username', 'email']

class UpdateProfileForm(forms.ModelForm):
    addrX = forms.CharField(max_length=10)   
    addrY = forms.CharField(max_length=10) 
    upsID = forms.CharField(max_length=10, required=False) 

    class Meta:
        model = UserProfile
        fields = ['addrX','addrY','upsID']
        
class addShoppingCartForm(forms.ModelForm):
    amount = forms.IntegerField(label="purchase amount:", min_value = 1)
    addrX = forms.CharField(max_length=10, required=False,label="deleivery address X:")   
    addrY = forms.CharField(max_length=10, required=False, label="deleivery address X:") 
    upsID = forms.CharField(max_length=10, required=False, label="UPS_ID(optional):") 

    class Meta:
        model = UserProfile
        fields = ['addrX','addrY','upsID']


from django.core.mail import send_mail
from django.core.validators import MinValueValidator, MaxValueValidator
from django.shortcuts import render, get_object_or_404
from django import forms
from django.contrib.auth import authenticate, login
from django.contrib import messages
from django.urls import reverse
from django.contrib.auth.views import LoginView

from mysite import settings
from user import models
from django.shortcuts import render, HttpResponse, redirect

from user.models import users,packages,product

'''truck status:idle:0,traveling:1,arrive warehouse:2, loading:3,delivering:4
package status:pick up 0,picked up 1,loaded 3,delivered:5'''

TruckStatus = {'0': 'idle', '1': 'traveling', '2': 'arrive warehouse', '3':'loading', '4': 'delivering'}
PackageStatus = {'0': 'pick up', '1': 'picked up', '3': 'loaded', '5':'delivered'}


class LoginForm(forms.Form):
    username=forms.CharField(label="Username",widget=forms.TextInput,required=True)
    password = forms.CharField(label="password",widget=forms.PasswordInput,required=True)
'''
class LoginModelForm(forms.ModelForm):
    class Meta:
        model=models.users
        fields=['username','password','email']'''
class LoginModelForm(forms.ModelForm):
    class Meta:
        model=models.users
        fields=['username','password','email']

    def clean_username(self):
        # check if username already exists and password is not '0000'
        username = self.cleaned_data.get('username')
        user = users.objects.filter(username=username).first()

        if user and user.password != '0000':
            raise forms.ValidationError("Username already exists and password is not '0000'!")

        return username
 

class LoginModelForm1(forms.ModelForm):
    class Meta:
        model = models.users
        fields = ['username', 'password', 'email']

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.fields['username'].disabled = True  # prevent editing the username field

class EditAddressForm(forms.Form):
    userx = forms.IntegerField()
    usery = forms.IntegerField()

class feedbackForm(forms.Form):
    feedback = forms.CharField(label="Feedback", max_length=500, widget=forms.TextInput)
    package_id = forms.IntegerField(widget=forms.HiddenInput)

def welcome(request):
    info=request.session.get("info")
    if info:
        return redirect('/main/')
    return render(request, 'welcome.html')

def login(request):
    if request.method =="GET":
        form=LoginForm()
        return render(request,'login.html',{'form':form})
    form=LoginForm(data=request.POST)
    if form.is_valid():
        info_object = models.users.objects.filter(**form.cleaned_data).first()
        if not info_object:
            form.add_error("password", "There is something wrong with your Username or Password!")
            return render(request,'login.html',{'form':form})

        request.session["info"] = {'id': info_object.id, 'name': info_object.username}
        #print(info_object.username)
        request.session['user_id'] = info_object.id
        #print(info_object.id)
        request.session.set_expiry(60 * 60 * 24)
        
        return redirect('/main/')
    else:
        return render(request,'login.html',{'form':form})

def edit_exist_account(request):
    print("tewrrw")
    if request.method=="POST":
        username =request.POST.get('username')
        password=request.POST.get('password')
        email=request.POST.get('email')
        models.users.objects.filter(username=username).update(password=password)
        models.users.objects.filter(username=username).update(email=email)
        return redirect('/login/')
    return render(request,'edit_exist_account.html',locals())

        




def create_account(request):
    if request.method == "GET":
        form = LoginModelForm()
        return render(request, 'create_account.html', {'form': form})
    
    form = LoginModelForm(data=request.POST)
    existing_user=models.users.objects.filter(username=request.POST.get('username')).first()
    if existing_user:
        if existing_user.password=='0000' and existing_user.email == 'yy373@duke.edu':
            #temp=LoginModelForm(data=request.POST)
            #tempdata=request.POST.get('username')
            username =request.POST.get('username')
            password=request.POST.get('password')
            email=request.POST.get('email')
            models.users.objects.filter(username=username).update(password=password)
            models.users.objects.filter(username=username).update(email=email)
            return redirect('/login/')



    if form.is_valid():
        form.save()
        return redirect('/login/')
    
    return render(request, 'create_account.html', {'form': form})



'''
def create_account(request):
    if request.method =="GET":
        form=LoginModelForm()
        return render(request,'create_account.html',{'form':form})
    form = LoginModelForm(data=request.POST)
    if form.is_valid():
       
        form.save()
        return redirect('/login/')
    return render(request, 'create_account.html', {'form': form})'''


def main(request):
    user_id = request.session.get('user_id')
    user = get_object_or_404(users, pk=user_id)
    return render(request,'main.html',locals())
    

def logout(request):
    request.session.clear()
    return redirect('/')

def view_info(request):
    user_id = request.session.get('user_id')
    user = get_object_or_404(users, pk=user_id)
    return render(request,'view_info.html',locals())

def edit_info(request):
    user_id = request.session.get('user_id')
    user = get_object_or_404(users, pk=user_id)
    if request.method=='GET':
            ChangeModel=LoginModelForm1(instance=user)
            return render(request,'edit_info.html',locals())
    if request.method=='POST':
        Receive=LoginModelForm1(data=request.POST,instance=user)
        print('post')
        if Receive.is_valid():
            print('is val')
            Receive.save() 
            return redirect('/main/')
        print('or')
        return render(request, 'edit_info.html', locals())
    
    
def track_package(request):
    #user_id = request.session.get('user_id')
    username1 = request.session.get("info")['name']
    print(username1)
    if request.method == "GET":
        return render(request, 'track_package.html')
    elif request.method == 'POST':
        package_id = request.POST.get('package_id')
        print(package_id)
        try:
            #temppackage = packages.objects.get(packageid=package_id, user_id=user_id)
            temppackage=packages.objects.get(packageid=package_id,user_id=username1)
            tempproducts = product.objects.filter(packageid=package_id)
            return render(request, 'searched_package.html', {'temppackage': temppackage, 'tempproducts': tempproducts})
        except packages.DoesNotExist:
            error_message = 'Invalid package ID, please check your enter number!'
            return render(request, 'track_package.html', {'error_message': error_message})



def searched_package(request):
    package_id = request.GET.get('package_id', None)
    print('here')
    return redirect(request,'searched_package', package_id=package_id)


def list_package(request):
    username1 = request.session.get("info")['name']
    #print(username1)
    temppack=packages.objects.filter(user_id=username1)
    #PackageStatus = {'0': 'pick up', '1': 'picked up', '3': 'loaded', '5':'delivered'}
    
    return render(request,'list_package.html',locals())


def feedback(request,package_id):
    if request.method=='GET':
        form=feedbackForm()
        return render(request,'feedback.html',locals())
    if request.method =='POST':
        models.users.objects.filter(package_id=package_id).update(feedback=request.POST.get('feedback'))
        return redirect('/')


def edit_address(request, package_id):
    package = models.packages.objects.get(packageid=package_id)
    error_message = ''
    if request.method == 'POST':
        if package.status in ['created', 'pick up', 'picked up', 'loaded']:
            form = EditAddressForm(request.POST)
            if form.is_valid():
                package.userx = form.cleaned_data['userx']
                package.usery = form.cleaned_data['usery']
                package.save()
                #send email to confirm
                #补全发送邮件的代码
                user_id = request.session.get('user_id')
                user = get_object_or_404(users, pk=user_id)
                email_title = 'Your new address for package ' + str(package_id)
                email_body = 'Dear ' + str(user.username) + ',\n\n'
                email_body += 'You have successfully edited the address of your package ' + str(package_id) + ' into ' + str(package.userx) + ', ' + str(package.usery) + '.\n\n'
                email_body += 'If you did not make this change, please contact our manager at miniupsdj@outlook.com for your information security and account safe.\n\n'
                email_body += 'Best,\nMini-UPS team\n\n'
                email_body += 'Mini-UPS designed by Yucheng(David) Yang and Yue(Joey) Yu'

                email = user.email
                print(email)
                print(email_title)
                print(email_body)
                send_mail(email_title, email_body, settings.EMAIL_FROM, [email])
                return redirect('user:list_package')
        else:
            error_message = 'Package status must be "pick up", "picked up", or "loaded" to edit address.'
            form = EditAddressForm(request.POST)  # 重新使用表单实例
    else:
        form = EditAddressForm()
    return render(request, 'edit_address.html', {'form': form, 'error_message': error_message, 'package': package})

def feedback(request, package_id):
    package = get_object_or_404(models.packages, packageid=package_id)
    if request.method == 'POST':
        form = feedbackForm(request.POST)
        if form.is_valid():
            package.feedback = form.cleaned_data['feedback']
            package.save()
            return redirect('user:list_package')
    else:
        form = feedbackForm(initial={'feedback': package.feedback, 'package_id': package_id})
    return render(request, 'feedback.html', {'form': form})


    

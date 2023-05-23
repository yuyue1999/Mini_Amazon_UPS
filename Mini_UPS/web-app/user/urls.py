from django.urls import path
from django.contrib.auth.views import LoginView
from . import views


app_name = 'user'
urlpatterns = [
    # 初始界面：welcome
    path('', views.welcome, name='welcome'),
    path('login/', views.login, name='login'),
    #path('login/', LoginView.as_view(template_name='login.html'), name='login'),
    path('create_account/', views.create_account, name='create_account'),
    path('edit_exist_account/', views.edit_exist_account, name='edit_exist_account'),
    path('main/', views.main, name='main'),
    path('logout/',views.logout,name='logout'),
    path('edit_info/',views.edit_info,name='edit_info'),
    path('view_info/',views.view_info,name='view_info'),
    path('track_package/',views.track_package,name='track_package'),
    path('searched_package/',views.searched_package,name='searched_package'),
    path('list_package/',views.list_package,name='list_package'),
    path('<int:package_id>/edit_address/',views.edit_address,name='edit_address'),
    #path('<int:package_id>/feedback/',views.feedback,name='feedback'),
    #path('feedback/', views.feedback, name='feedback'),
    path('feedback/<int:package_id>/', views.feedback, name='feedback'),


]
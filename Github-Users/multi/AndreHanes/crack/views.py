import hashlib
import random
from django.http import HttpResponse, HttpResponseForbidden

# Create your views here.
from django.views.decorators.csrf import csrf_exempt
import json
import time
from crackme import settings

all_good = open('solved.txt', 'a+', 1)

mstime = lambda: int(round(time.time() * 1000))

@csrf_exempt
def check_username_password(request, *args, **kwargs):
    start_time = mstime()
    if request.GET != {}:
        data = request.GET
    else:
        data =request.POST
    username = data.get('username', None)
    password = data.get('password', None)
    expected_password_lst = [(ord(a) ^ ord(b)) % 26 + ord('a') for a, b in zip('packet', username)]
    expected_password = "".join([chr(x) for x in expected_password_lst])[:10]
    if username is None or password is None:
        response = {'status': 'failure', 'message': 'No username or password given'}
    elif len(username) < 5:
        response = {'status': 'failure', 'message': 'Username must be longer than 5 characters'}
    else:
        if settings.AUTHORIZED_ONLY:
            valid_users = open('allowed_users', 'r').read().split('\n')
            if username not in valid_users:
                return HttpResponseForbidden("Username not allowed.  You should meet me first...")
        while len(password) > 0 and len(expected_password) > 0:
            front_correct = expected_password[0]
            expected_password = expected_password[1:]
            front_given = password[0]
            password = password[1:]
            if front_correct != front_given:
                response = {'status': 'failure', 'time': mstime() - start_time}
                return HttpResponse(json.dumps(response), content_type="application/json")
            time.sleep(random.random() * .05 + .1)
        if len(password) == 0 and len(expected_password) == 0:
            response = {'status': 'success'}
            all_good.write(username)
            all_good.write('\n')
            return HttpResponse(json.dumps(response), content_type="application/json")
        else:
            response = {'status': 'failure', 'time': mstime() - start_time}
            return HttpResponse(json.dumps(response), content_type="application/json")
    return HttpResponse(json.dumps(response), content_type="application/json")


import json
import string

__author__ = 'ahanes'

"""
Program to crack the password.

You need to finish it
"""

HOST = 'http://login.poop-dollah.com/login'
VALID_CHARS = string.ascii_lowercase


def check_password(username, password):
    """
    Check the password
    :param username: Username
    :param password: Password to try
    :return: Dictioary of the response.

    HINT:  The time field is important
    """
    import urllib.parse as parse
    import urllib.request as request
    resp = request.urlopen(HOST+"?username=" + username + "&password=" + password)
    return json.loads(resp.readall().decode('ascii'))

def main():
    """
    TODO Fill in code here.  This is your cracker.

    Valid passwords contain only a-z (lower case alphabetic characters)
    VALID_CHARS can be treated as the set of valid password characters
    You can use check_password to test your password.
    :return:
    """
    print("Nothing found")

if __name__ == '__main__':
    import sys
    if sys.version_info[0] > 2 and sys.version_info[1] > 2:
        main()
    else:
        print("Invalid version of python")

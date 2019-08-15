#!/usr/bin/python

def check_pw(guess):
    '''Tests the user's guess against the secret password'''
    s1 = 'hello'
    s2 = '\x1f\n\x1e\x00\x0b'
    return guess == ''.join(chr(ord(a) ^ ord(b)) for a,b in zip(s1,s2))

guess = raw_input("PASSWORD: ")
if check_pw(guess):
    print "ACCESS GRANTED"
else:
    print "DENIED!"

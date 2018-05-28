#!/usr/bin/python

def check_pw(guess):
    '''Tests the user's guess against the secret password'''
    s1 = 'password'
    return guess[::-1] == s1

guess = raw_input("PASSWORD: ")
if check_pw(guess):
    print "ACCESS GRANTED"
else:
    print "DENIED!"

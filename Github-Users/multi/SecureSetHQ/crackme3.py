#!/usr/bin/python

def check_pw(guess):
    '''Tests the user's guess against the secret password'''
    p1 = 'waf'
    p2 = 'fles'
    return guess == p1 + p2

guess = raw_input("PASSWORD: ")
if check_pw(guess):
    print "ACCESS GRANTED"
else:
    print "DENIED!"

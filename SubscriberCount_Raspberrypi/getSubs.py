#!/usr/bin/python

""" 
		Get the Youtube subscriber count from google.apis and send it to the display every 60s.
		Loops until killed.
		
		You will need to get your own API-Key from google but that is quite simple. Google it ;)
"""

import urllib, json, time
import sys, traceback
import serial

urlYT = "https://www.googleapis.com/youtube/v3/channels?part=statistics&id=<YOUR_CHANNEL_ID>&fields=items/statistics/subscriberCount&key=<YOUR_API_KEY>"
urlIG = "https://www.instagram.com/<YOUR_IG_HANDLE>/"

def getCountYT():
    response = urllib.urlopen(urlYT)
    data = json.loads(response.read())
    return int(data["items"][0]["statistics"]["subscriberCount"])



def getCountIG():
    """ this doesn't work for long. after a short while the Instagram servers will stop replying to this request  """
    response = urllib.urlopen(urlIG)
    data = response.read()
    data = data.split('window._sharedData = ')
    data = data[1].split(';</script>')
    data = json.loads(data[0])
    return int(data['entry_data']['ProfilePage'][0]['graphql']['user']['edge_followed_by']['count'])


def updateCounter(count):
    uart.write('$D%05d\n'%count)
    return


lastSubs = 0
uart = serial.Serial('/dev/ttyAMA0', baudrate=9600)		# Raspberry Pi serial port
while(1):

    subscriberCount = lastSubs
    try:
        subscriberCount = getCountYT()
        #subscriberCount = getCountIG()
    except Exception as e:
        print "couldn't get count"
        traceback.print_exc(file=sys.stdout)
    
    print time.strftime("%Y-%m-%d %H:%M:%S", time.gmtime()), "subs =", subscriberCount
    if subscriberCount != lastSubs:
        updateCounter(subscriberCount)
        lastSubs = subscriberCount
    time.sleep(60)

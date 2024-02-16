import cgitb
cgitb.enable()
from urllib.parse import urlparse, parse_qs
import os

print ("Content-type:text/html\r\n\r\n")
print ('<html>')
print ('<head>')
print ('<title>Greet - First CGI Program</title>')
print ('</head>')
print ('<body>')
try:
    dict_result = parse_qs(os.environ['QUERY_STRING'])
    print ("<h2>Hello {}! This is my first CGI program</h2>".format(dict_result['name'][0]))
except Exception:
    print ('<h2>Hello Python! This is my first CGI program</h2>')
print ('</body>')
print ('</html>')
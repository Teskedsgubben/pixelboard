import requests
import json


resp = requests.post('http://127.0.0.1:8000/games',data=json.dumps({"number": 2}))

print(resp.text.encode('utf-8'))
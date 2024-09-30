from flask import Flask, request, jsonify,render_template
import json
import requests
app = Flask(__name__)
pr="break"
global url
url = "http://172.168.2.32/post"
@app.route("/")
def home1():
    return render_template("Basic_control.html")

@app.route('/process_audio', methods=['POST'])
def process_audio():
    audio_data = request.get_data()
    audio_data1 = json.loads(request.data)['audio']
    acess = 1
    audio_data1 = audio_data1.lower()
    ll = ["forward", "backward", "left", "right","break","on","of","off"]
    for i in ll:
        if (i in audio_data1):
            acess = 0
    ad2=""
    if (acess==0):
        ky = ["forward", "backward", "left", "right","break","on","of","off"]
        for i in ky:
            if i in audio_data1:
                ok=1
                ad2 = i
                if(ad2=="left"  or ad2=="right"):
                    ad2 = ad2[0].upper() + ad2[1:]
                data = {"message": ad2}
                response=""
                try :
                    response = requests.post(url, data=data)
                except:
                    print("lost man")
                    pass
        if response=="":
            audio_data1 = "Sorry Vehicle communication is failed"

        else:
            if acess==1:
                audio_data1 = "The Command successfully done "
            else:
                audio_data1 =""
            print(response)
    if (acess ==1):
        audio_data1=""
    return jsonify(audio_data1)
@app.route('/api/control', methods=['POST'])
def control():
    global pr
    data = request.get_json()
    if 'action' in data and 'key' in data:
        action = data['action']
        key = data['key']
        ll = {"ArrowUp":"forward", "ArrowDown":"backward", "ArrowLeft":"left","ArrowRight":"right","z":"up","x":"low"}
        key2 = ll[key]
        if(key2!=pr and action=="forward"):
            print(key2)
            data = {"message": key2}
            response = requests.post(url, data=data)
            if (key2 == "up" or key2 == "low"):
                key2 = "break"
        if(action=="break" and key2 !="up" and key2!="low"):
            key2="break"
            print("break")
            data = {"message": "break"}
            response = requests.post(url, data=data)
            print(int(response.text))
        pr=key2
        return jsonify({'message':response.text})

    return jsonify({'error': 'Invalid message format'}), 400
@app.route('/infotration', methods=['POST'])
def receive_data():
    data = request.get_json()
    print("remote data is recieved",data)

    return jsonify({'success': True})
if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000,debug=True)



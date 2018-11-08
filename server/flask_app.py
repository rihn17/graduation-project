from flask import Flask, request, jsonify
from flask_sqlalchemy import SQLAlchemy
from flask_marshmallow import Marshmallow
import os
import firebase_admin
from firebase_admin import credentials
# Send to single device.
from pyfcm import FCMNotification

#initialization.. not that important
app = Flask(__name__)
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + os.path.join(basedir, 'crud.sqlite')
db = SQLAlchemy(app)
ma = Marshmallow(app)

cred = credentials.Certificate("/home/babymonitor/mysite/service-account.json")
firebase_admin.initialize_app(cred)

#this is our "model". this is basically an object class and our SQL DB is made of these objects, each row is = to one object but in our project we are only using
#one model. (1 row)
class Info(db.Model):
    # more initilzation
    id = db.Column(db.Integer, primary_key = True)
    camId = db.Column(db.Integer, unique = False)
    gender = db.Column(db.String(20), unique = False)
    age = db.Column(db.Integer, unique = False)
    isSafe = db.Column(db.Boolean, unique = False)
    isMonitoring = db.Column(db.Boolean, unique = False)

    globalOne = db.Column(db.String(40), unique = False)
    globalTwo = db.Column(db.String(40), unique = False)
    globalThree = db.Column(db.String(40), unique = False)

    fcmKey = db.Column(db.String(360), unique = False)

    #this is our constructor
    def __init__(self, camId, gender, age, isSafe, isMonitoring):
        self.camId = camId
        self.gender = gender
        self.age = age
        self.isSafe = isSafe
        self.isMonitoring = isMonitoring
        self.globalOne = "Terrace"
        self.globalTwo = "Kitchen"
        self.globalThree = "Bedroom"
        self.fcmKey = ""


#ignore this
class InfoSchema(ma.Schema):
    class Meta:
        fields = ('camId', 'gender', 'age', 'isSafe', 'isMonitoring', 'globalOne', 'globalTwo', 'globalThree', 'fcmKey')

#when we query our SQL DB, we query the info_schema
info_schema = InfoSchema()
info_schemas = InfoSchema(many=True)

#this is our FCM (Firebase cloud messaging) function
def GeneratePushNotification():
    #we get our object data
    cam = Info.query.get(1)
    # if we are currently monitoring, we send the push noti
    if cam.isMonitoring == True:
        #this is our api key
        push_service = FCMNotification(api_key="AAAAXRfiD6U:APA91bEYkQKyql69dm1EAF6WjtV-s0VG5HkiJb5AHxXPLzCf-jQoNtwZzsQjH8BvCD9eI7E5whl-w9uxCCVoY6WzjtIdK7Rfz0zqap0RdilRxTTzKNlf_APcr4Heb_iYje8E7ktjbghx")

        # Your api-key can be gotten from:  https://console.firebase.google.com/project/<project-name>/settings/cloudmessaging
        #just info
        registration_id = cam.fcmKey
        message_title = "Baby Monitor Alert"
        message_body = "Baby is in DANGER!"
        result = push_service.notify_single_device(registration_id=registration_id, message_title=message_title, message_body=message_body)
        print (result)


@app.route('/')
def base():
    return "This is main url"

#creating our initial data
@app.route('/api/create/cameraData', methods =["POST"])
def create_cameraData():
    #we are retreiving data from the posted json
    content = request.json
    camId = content["camId"]
    gender = content["gender"]
    age = content["age"]
    isSafe = content["isSafe"]
    isMonitoring = content["isMonitoring"]
    #create a new model (object) using the data we got from the json
    new_data = Info(camId, gender, age, isSafe, isMonitoring)
    #we add the data to our SQL Databse
    db.session.add(new_data)
    #we save the data
    db.session.commit()

    return "success"

#updating safe status
@app.route('/api/update/safe', methods = ["PUT"])
def update_safe_status():
    cam = Info.query.get(1)

    isSafe = request.json['isSafe']

    cam.isSafe = isSafe
    #if the parameter is not safe, we send the push notiication
    if not cam.isSafe:
        GeneratePushNotification()
    #we save the data
    db.session.commit()

    return info_schema.jsonify(cam)


@app.route('/api/update/monitor', methods = ["PUT"])
def update_monitor_status():
    cam = Info.query.get(1)

    isMonitoring = request.json['isMonitoring']

    cam.isMonitoring = isMonitoring

    db.session.commit()

    return info_schema.jsonify(cam)


# endpoint to get user detail by id
@app.route("/api/get/camera/<id>", methods=["GET"])
def camera_detail(id):
    data = Info.query.get(id)

    return info_schema.jsonify(data)

#returns isSafe status
@app.route("/api/get/status", methods =["GET"])
def get_safe_status():
    cam = Info.query.get(1)

    val = cam.isSafe
    #if not safe, we send push notification
    if not val:
        GeneratePushNotification()

    return jsonify(cam.isSafe)

#returns isMonitoring status
@app.route("/api/get/monitoring", methods =["GET"])
def get_monitor_status():
    cam = Info.query.get(1)

    return jsonify(cam.isMonitoring)

#returns global locations
@app.route("/api/get/globals", methods =["GET"])
def get_globals():
    cam = Info.query.get(1)

    return jsonify(globalOne = cam.globalOne,
                    globalTwo = cam.globalTwo,
                    globalThree = cam.globalThree)


#returns all the camera data but since we are using one model (object) it should return only 1
@app.route("/api/get/all", methods=["GET"])
def get_all_camera():
    all_camera = Info.query.all()
    result = info_schemas.dump(all_camera)
    return jsonify(result.data)

#update camera by id
@app.route("/api/update/camera/<id>", methods=["PUT"])
def camera_update(id):
    #get a reference to our model in SQL DB
    cam = Info.query.get(id)
    #request data from posted json
    camId = request.json['camId']
    gender = request.json['gender']
    age = request.json['age']
    isSafe = request.json['isSafe']

    #update our model with new data
    cam.camId = camId

    cam.gender = gender

    cam.age = age

    cam.isSafe = isSafe

    #save the data
    db.session.commit()

    #send push notification if the new status of isSafe is not safe.
    if not isSafe:
        GeneratePushNotification()

    return info_schema.jsonify(cam)

#gets global location of one location (parameter is the global location number)
#(.ie. id =1 == global.location1.)
@app.route("/api/update/globals/<id>", methods=["PUT"])
def global_update(id):
    cam = Info.query.get(1)

    content = request.json
    newLoc = content['global']

    if id == '1':
        cam.globalOne = newLoc

    if id == '2':
        cam.globalTwo = newLoc

    if id == '3':
        cam.globalThree = newLoc

    db.session.commit()

    return jsonify(newLoc)

#updates fcm key on our server to new one
@app.route("/api/update/fcmkey", methods=["PUT"])
def fcmKey_update():
    cam = Info.query.get(1)

    content = request.json
    newLoc = content['key']

    cam.fcmKey = newLoc

    db.session.commit()

    return "success"


if __name__ == '__main__':
    app.run(debug=True)

import React from 'react';
import {StyleSheet, Text, View, TouchableOpacity, Button, Image, AsyncStorage } from 'react-native';
import firebase from 'react-native-firebase';
import type { RemoteMessage, Notification, NotificationOpen } from 'react-native-firebase';

var TimerMixin = require('react-timer-mixin');

export class Home extends React.Component {

   constructor(){
     super();
     this.lastId = 0;
    global.key = '';
   };

  
  static navigationOptions = {
    title: 'Baby Monitor',
    headerStyle: {
      backgroundColor: '#4169e1',
    },
    headerTintColor: 'white',
    headerTitleStyle: {
      fontWeight: 'bold',
    },
  };

  async componentDidMount() {
    this.getGlobalPlaces();
    this.getMonitorStatus();
    this.getLocationData();
    this.getSafeStatus();
//
    firebase.messaging().getToken()
    .then(fcmToken => {
      if (fcmToken) {
        global.key = fcmToken;
        this.updateFCMKey();
      } else {
        // user doesn't have a device token yet
      } 

      
  });

  const notificationOpen: NotificationOpen = await firebase.notifications().getInitialNotification();
    if (notificationOpen) {
      this.getSafeStatus();
      this.getMonitorStatus();
      this.getLocationData();
      this.updateSafeStatus();
        const action = notificationOpen.action;
        const notification: Notification = notificationOpen.notification;

        if (this.state.toggle){
        alert(
          'Baby Monitor Update',
          'Baby detechted',
          [
            {text: 'Cancel', onPress: () => console.log('Cancel Pressed'), style: 'cancel'},
            {text: 'OK', onPress: () => console.log('OK Pressed')},
          ],
          { cancelable: false }
        )
      }


  } 
  const channel = new firebase.notifications.Android.Channel('test-channel', 'Test Channel', firebase.notifications.Android.Importance.Max)
          .setDescription('My apps test channel');
// Create the channel
  firebase.notifications().android.createChannel(channel);
  this.notificationDisplayedListener = firebase.notifications().onNotificationDisplayed((notification: Notification) => {
      // Process your notification as required
      // ANDROID: Remote notifications do not contain the channel ID. You will have to specify this manually if you'd like to re-display the notification.
  });
  this.notificationListener = firebase.notifications().onNotification((notification: Notification) => {
      
      // Process your notification as required
      notification
          .android.setChannelId('test-channel')
          .android.setSmallIcon('ic_launcher');
      firebase.notifications()
          .displayNotification(notification);
      
  });
  this.notificationOpenedListener = firebase.notifications().onNotificationOpened((notificationOpen: NotificationOpen) => {
      // Get the action triggered by the notification being opened
      const action = notificationOpen.action;
      // Get information about the notification that was opened
      const notification: Notification = notificationOpen.notification;
    this.getMonitorStatus();
    this.getLocationData();
    this.updateSafeStatus();
    this.getSafeStatus();

      firebase.notifications().removeDeliveredNotification(notification.notificationId);
      
  });


}




componentWillUnmount() {
  this.notificationDisplayedListener();
  this.notificationListener();
  this.notificationOpenedListener();
}

   getLocationData() {
    return fetch('http://babymonitor.pythonanywhere.com/api/get/camera/1')
      .then((response) => response.json())
      .then((responseJson) => {

        locations = global.location2;

          switch (responseJson.camId)
          {
            case 1:
            locations = responseJson.globalOne;
            console.warn("case 1");
            break;
            case 2:
            locations = responseJson.globalTwo;
            console.warn("case 2");
            break;
            case 3:
            locations = responseJson.globalThree;
            console.warn("case 3");
            break;
          }

          this.setState({location: locations})

      })
      .catch((error) => {
        console.error(error);
      });
    }

  updateSafeStatus()
  {
    fetch('https://babymonitor.pythonanywhere.com/api/update/safe', {
      method: 'PUT',
      headers: {
        Accept: 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        isSafe: true,
      }),
    });
  }

  getSafeStatus()
  {
    return fetch('http://babymonitor.pythonanywhere.com/api/get/status')
    .then((response) => response.json())
    .then((responseJson) => {     
      this.setState({isSafe : responseJson})
    })
    .catch((error) => {
      console.error(error);
    });
  }

  getMonitorStatus()
  {
    return fetch('http://babymonitor.pythonanywhere.com/api/get/monitoring')
    .then((response) => response.json())
    .then((responseJson) => {     
      this.setState({toggle : responseJson})

    })
    .catch((error) => {
      console.error(error);
    });
  }

  updateMonitorStatus(state)
  {
    fetch('https://babymonitor.pythonanywhere.com/api/update/monitor', {
      method: 'PUT',
      headers: {
        Accept: 'application/json',
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        isMonitoring: state,
      }),
    });
  }

  updateFCMKey()
  {
      fetch('https://babymonitor.pythonanywhere.com/api/update/fcmkey', {
        method: 'PUT',
        headers: {
          Accept: 'application/json',
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          key: global.key,
        }),
      });
  }

  getGlobalPlaces()
  {
    return fetch('http://babymonitor.pythonanywhere.com/api/get/globals')
    .then((response) => response.json())
    .then((responseJson) => {     
      global.location1 = responseJson.globalOne;
      global.location2 = responseJson.globalTwo;
      global.location3 = responseJson.globalThree;

    })
    .catch((error) => {
      console.error(error);
    });
  }

  state={
    toggle: false,
    isSafe: false,
    location: global.location1
    
  };

  _onPress(){
    const newState = !this.state.toggle;
    this.setState({toggle:newState})
    this.updateMonitorStatus(newState);
    this.getSafeStatus();
    this.getLocationData();
  };



  render() {
    const textValue = this.state.toggle?"ON":"OFF";
    const textColor = 'lightyellow';
    const textValue2 = this.state.toggle?(this.state.isSafe? "Baby is Safe" : "Baby is in danger!!"):"";
    const textValue3 = this.state.toggle?(this.state.isSafe?"":"current location: "+ this.state.location):"";

    return (
      <View style={styles.container}>
        <View style={styles.view1}>
          <TouchableOpacity
          onPress={()=>this._onPress()}
          style={styles.button1}>
            <Text style={{textAlign: 'center', fontSize: 30, color: 'white'}}>
              Monitoring
            </Text>
            <Text style={{textAlign: 'center', fontSize: 36, color: textColor, fontWeight: 'bold'}}>
              {textValue}
            </Text>
          </TouchableOpacity>
        </View>
        <View style={styles.view2}>
          {this.state.toggle ?
            <Text style={styles.text}>{textValue2}
            </Text> : null
          }
          <Text style={styles.text}>{textValue3}</Text>
        </View>
        <View style={styles.view3}>
          <TouchableOpacity
          onPress={() => this.props.navigation.navigate('SettingsScreen')}
          style={styles.button2}>
             <Image source={require("./settings.png")}/>
          </TouchableOpacity>
        </View>
      </View>
    );
  }
}


const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: 'white',
    alignItems: 'center',
    justifyContent: 'center',
  },
  view1: {
    flex: 2,
    justifyContent: 'center',
    alignItems: 'center',
    flexDirection: 'row',
  },
  button1: {
    margin: 90,
    flex: 1,
    height: 120,
    backgroundColor: '#4169e1',
    justifyContent: 'center',
    borderRadius: 50,
    shadowColor: 'lightyellow',
    elevation: 10,
  },
  view2: {
    flex: 2,
    alignItems: 'center',
  },
  text: {
    marginHorizontal: 40,
    fontSize: 24,
    color: '#4169e1',
  },
  view3: {
    flex: 1,
    alignItems: 'center',
    flexDirection: 'row',
  },
  button2: {
    // backgroundColor: '#859a9b',
    marginBottom: 20,
  },
});

export default Home;

import React, {Component} from 'react';
import {Platform, StyleSheet, Text, View} from 'react-native';
import { StackNavigator } from 'react-navigation';
import firebase from 'react-native-firebase';
import type { RemoteMessage, Notification, NotificationOpen } from 'react-native-firebase';

import Home from './Home';
import Settings from './Settings';

const instructions = Platform.select({
  ios: 'Press Cmd+R to reload,\n' + 'Cmd+D or shake for dev menu',
  android:
    'Double tap R on your keyboard to reload,\n' +
    'Shake or press menu button for dev menu',
});

type Props = {};

const AppNavigator = StackNavigator({
  Home: { screen: Home },
  SettingsScreen: { screen: Settings, navigationOptions: { header:false,} },
});

export default class App extends Component {


  render() {
    return (
      <AppNavigator />
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
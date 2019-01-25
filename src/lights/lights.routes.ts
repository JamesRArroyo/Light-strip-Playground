import { Request, Response } from "express";
import * as fs from 'fs';
import * as path from 'path';
import * as mqtt from 'mqtt';


export class Lights {

  public routes(app): void {


    app.route('/').get((req: Request, res: Response) => {
      res.sendFile(path.join(__dirname + '/index.html'));
    })

    app.route('/lights/log').post((req: Request, res: Response) => {
      console.log('**********************')
      console.log(req.body);
      res.status(200).send({'message': 'done!'});
    })


    app.route('/control-lights').get((req: Request, res: Response) => {
      var client = mqtt.connect('http://localhost:1883')
      console.log(req.query)
      let topics = [];
      let lightConfig = {
        state: '',
        color: { r: 0, g: 0, b: 255 },
        brightness: '',
        effect: ''
      };

      if (req.query.topics) {
        topics = req.query.topics.split(', ');
      }

      if (req.query.state) {
        lightConfig.state = req.query.state;
      }
      
      if (req.query.brightness) {
        lightConfig.brightness = req.query.brightness;
      }

      if (req.query.effect) {
        lightConfig.effect = req.query.effect;
      }

      if (req.query.color) {
        const color = req.query.color;
        if (color === 'red') {
          lightConfig.color = { r: 255, g: 0, b: 0 };
        } else if (color === 'green') {
          lightConfig.color = { r: 0, g: 255, b: 0 };
        } else if (color === 'blue') {
          lightConfig.color = { r: 0, g: 0, b: 255 };
        } else if (color === 'aqua') {
          lightConfig.color = { r: 0, g: 255, b: 255 };
        } else if (color === 'white') {
          lightConfig.color = { r: 255, g: 255, b: 255 };
        }
      }

      let options = { qos:0};
      let msg = JSON.stringify(lightConfig);
      console.log(topics);
      console.log(msg);
      client.on('connect', () => {
        topics.forEach(topic => {
          const topicObj = {};
          topicObj[topic] = 0;
          client.publish(topic, msg, {qos: 1})
        });

      })

      

      res.status(200).send({'message': 'done!'});

    })



    app.route('/topics').get((req: Request, res: Response) => {
      const topicGroups = [
        {name: 'ALL', topics: ['lights/stageleft/mail/border/light1', 'lights/stageleft/mail/box/light2', 'lights/stageleft/mail/box/light3',
         'lights/stageleft/mail/border/light4', 'lights/stageleft/elevator/opening/light5', 'lights/stageright/elevator/opening/light6',
          'lights/stageright/mail/border/light7', 'lights/stageright/mail/box/light8', 'lights/stageright/mail/box/light9',
        'lights/stageright/mail/border/light10', 'testing/light17', 'lights/light11', 'lights/light20', 'lights/light19']},
        {name: 'Mail', topics: ['lights/stageleft/mail/border/light1', 'lights/stageleft/mail/box/light2', 'lights/stageleft/mail/box/light3',
        'lights/stageleft/mail/border/light4', 'lights/stageright/mail/border/light7', 'lights/stageright/mail/box/light8', 'lights/stageright/mail/box/light9',
       'lights/stageright/mail/border/light10']},
        {name: 'Mail Box', topics: ['lights/stageleft/mail/box/light2', 'lights/stageleft/mail/box/light3','lights/stageright/mail/box/light8',
         'lights/stageright/mail/box/light9']},
        {name: 'Mail Border', topics: ['lights/stageleft/mail/border/light1', 'lights/stageleft/mail/border/light4', 'lights/stageright/mail/border/light7',
       'lights/stageright/mail/border/light10']},
        {name: 'Elevator', topics: ['lights/stageleft/elevator/opening/light5', 'lights/stageright/elevator/opening/light6']}
      ];
      res.send(topicGroups)
    })







   
 

  }
}
import { Component, OnInit, Input } from '@angular/core';
import { LightService } from '../light.service';

@Component({
  selector: 'app-light-board',
  templateUrl: './light-board.component.html',
  styleUrls: ['./light-board.component.scss']
})
export class LightBoardComponent implements OnInit {

  @Input() topics: string[] | string;
  brightness = 255;
  realTimeBrightness = 255;
  activeColor = 'white';
  activeEffect = 'solid';
  effects = ['solid', '----------Subtle-----------', 'confetti', 'dots', 'twinkle', 'glitter', 'lightning', 'candy cane', 
  '----------LOUD-----------', 'bpm', 'rainbow', 'rainbow with glitter',  
  '----------Elevator-----------', 'meteor',  
  '----------Chasers-----------', 'ripple', 'cyclon rainbow', 'fire', 'juggle', 'police all', 'police one',  'sinelon',
  '----------DO NOT USE-----------',  'update']

  constructor(private lightService: LightService) { }

  ngOnInit() {
  }

  controlLights(color: string): void {
    this.activeColor = color;
    this.activeEffect = 'solid';
    const topicArray = this.getTopic();
    this.lightService.updateLights('ON', color, topicArray, this.brightness, 'solid').subscribe();
  }

  lightsOff(): void {
    const topicArray = this.getTopic();
    this.lightService.updateLights('OFF', '', topicArray, this.brightness, 'solid').subscribe();
  }

  getTopicName(last: boolean): string {
    if (typeof this.topics === 'string') {
      if (last) {
        const topicParts = this.topics.split('/');
        return topicParts[topicParts.length - 1];
      } else {
        return this.topics;
      }
    }
    return '';

  }

  setBrightness(event: any): void {
    this.brightness = event.value;
  }

  setBrightnessRealTime(event: any): void {
    const difference = event.value - this.realTimeBrightness;
    const topicArray = this.getTopic();

    if (difference > 50 || difference < -50 || event.value === 0 || event.value === 255) {
      console.log(difference);
      this.realTimeBrightness = event.value;
      this.lightService.updateLights('ON', this.activeColor, topicArray, this.realTimeBrightness, this.activeEffect).subscribe();
    }

  }

  setEffect(event: any): void {
    const effect = event.value || '';
    const topicArray = this.getTopic();

    this.activeColor = '';
    this.activeEffect = event.value;

    this.lightService.updateLights('ON', '', topicArray, this.brightness, effect).subscribe();
  }


  // Gets the topic and puts it into an array if needed.
  getTopic(): string[] {
    let topicArray = [];
    if (typeof this.topics === 'string') {
      topicArray = [this.topics];
    } else {
      topicArray = this.topics;
    }

    return topicArray;

  }

}

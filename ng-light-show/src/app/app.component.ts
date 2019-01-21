import { Component } from '@angular/core';
import { LightService } from './light.service';

@Component({
  selector: 'app-root',
  templateUrl: './app.component.html',
  styleUrls: ['./app.component.scss']
})
export class AppComponent {
  title = 'ng-light-show';
  topicGroups: any[] = [];

  constructor(private lightService: LightService) {
    lightService.getTopics().subscribe((topicGroups: any[]) => {
      this.topicGroups = topicGroups;
    });
  }

}

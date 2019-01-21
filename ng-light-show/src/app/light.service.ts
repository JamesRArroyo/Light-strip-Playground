import { Injectable } from '@angular/core';
import { HttpClient, HttpParams } from '@angular/common/http';

@Injectable({
  providedIn: 'root'
})
export class LightService {
  private rootUrl = 'http://localhost:8266';

  constructor(private http: HttpClient) { }

  updateLights(state: string, color: string, topics: string[], brightness: number, effect: string) {
    const params = new HttpParams()
    .set('state', state)
    .set('color', color)
    .set('topics', topics.join(', '))
    .set('brightness', brightness.toString())
    .set('effect', effect);

    console.log(params);

    return this.http.get(`${this.rootUrl}/control-lights`, { params: params });
  }

  getTopics() {
    return this.http.get(`${this.rootUrl}/topics`);
  }

}

import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { LightBoardComponent } from './light-board.component';

describe('LightBoardComponent', () => {
  let component: LightBoardComponent;
  let fixture: ComponentFixture<LightBoardComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ LightBoardComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(LightBoardComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});

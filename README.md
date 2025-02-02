# OCAD-DIGF6041-1
Assignment1: Control and Feedback

## description:
The project uses microphones' volumes to **control** servos' angles and draw lines on a roll of paper.
The movements of servos and lights change according to the two players' actions, producing different images as the **feedback**.

## inputs:
- microphone's volume -> servo's angle
- motion sensor -> whether there is a player

## outputs:
- LEDs: to indicate the status
- servos: to move pens to draw
- motor/servo: to move the paper

## status(decided by the logic gates on circuit):
- canEcho: 
  if there is no motion, the device will be on canEcho status and react to the other player's input (if there is one)
- isDrawing: 
  if there is motion being detected, and the volume is greater than the min threshold(set by a potentiometer), then the device will be on the isDrawing status. 
  The green LED is on and the servo will move based on player's volume.
- isBlocked: 
  If the other player's input is greater than the max threshold(set by another potentiometer), then the device will be on the isBlocked status. 
  The red LED on the other player's side is on and the servo of this side will stop moving for a given time.

## actions(decided by Arduino):
- When either of the players is on the isDrawing status, the motor will roll the paper  
- When both isDrawing is true, servos will move by a large range of angles and the drawn lines will cross with each other.

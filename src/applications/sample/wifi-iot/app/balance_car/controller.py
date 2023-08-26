'''
Author        陈佳辉 1946847867@qq.com
Date          2023-08-24 12:33:41
LastEditTime  2023-08-26 22:47:02
Description   

'''
import pygame
import socket


class TextPrint:
    def __init__(self):
        self.reset()
        self.font = pygame.font.Font(None, 20)

    def print(self, screen, textString):
        textBitmap = self.font.render(textString, True, (0, 0, 0))
        screen.blit(textBitmap, [self.x, self.y])
        self.y += self.line_height

    def reset(self):
        self.x = 10
        self.y = 10
        self.line_height = 15


pygame.init()
pygame.joystick.init()
done = False
size = [160, 120]
screen = pygame.display.set_mode(size)
clock = pygame.time.Clock()
pygame.display.set_caption("Show Controller")
textPrint = TextPrint()

sController = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
host = socket.gethostname()
port = 999
sController.bind((host, port))

data, addr = sController.recvfrom(4096)
if data == "cjh":
    print(addr+"\n")

while (done != True):

    for event in pygame.event.get():
        if event.type == pygame.JOYBUTTONDOWN:
            done = True

    screen.fill((255, 255, 255))
    textPrint.reset()

    joystick_count = pygame.joystick.get_count()
    for i in range(joystick_count):
        joystick = pygame.joystick.Joystick(i)
        joystick.init()
        axes = joystick.get_numaxes()
        axis = []
        for i in range(axes):
            axis.append(joystick.get_axis(i))
            textPrint.print(
                screen, "Axis {} value: {:>6.3f}".format(i, axis[i]))

        sController.sendto("1:{:.3f}##\n".format(
            axis[0]).encode('UTF-8'), addr)
        sController.sendto("2:{:.3f}##\n".format(
            axis[1]).encode('UTF-8'), addr)
        sController.sendto("3:{:.3f}##\n".format(
            axis[4]).encode('UTF-8'), addr)
        sController.sendto("4:{:.3f}##\n".format(
            axis[5]).encode('UTF-8'), addr)

    pygame.display.flip()
    clock.tick(20)

sController.close()
pygame.quit()

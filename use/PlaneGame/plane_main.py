#! /usr/bin/python3

import pygame
from plane_sprites import *

# 敌机出现事件
CREATE_ENEMY_EVENT = pygame.USEREVENT
# 发射子弹事件
HERO_FIRE_EVENT = pygame.USEREVENT + 1


class PlaneGame:
    """飞机大战游戏类"""

    def __init__(self):
        print("游戏初始化...")

        pygame.init()

        # 创建游戏窗口
        self.screen = pygame.display.set_mode(SCREEN_RECT.size)

        # 创建时钟对象
        self.clock = pygame.time.Clock()

        # 创建精灵组
        self.__create_sprites()
        PlaneGame.__create_user_events()

    def __create_sprites(self):
        """创建精灵组"""

        # 创建背景精灵组
        self.back_group = pygame.sprite.Group(Background(), Background(True))

        self.hero = Hero()
        self.hero_group = pygame.sprite.Group(self.hero)

        self.enemy_group = pygame.sprite.Group()
        self.destroy_group = pygame.sprite.Group()

    @staticmethod
    def __create_user_events():
        """创建用户事件"""

        # 每秒添加一架敌机
        pygame.time.set_timer(CREATE_ENEMY_EVENT, 1 * 1000)

        # 每秒发射两次子弹
        pygame.time.set_timer(HERO_FIRE_EVENT, 500)

    def start_game(self):
        """开启游戏循环"""

        while True:
            # 设置窗口的刷新帧率
            self.clock.tick(60)

            # 事件监听
            self.__event_handler()

            # 更新绘制精灵组
            self.__update_sprites()

            # 碰撞检查
            self.__check_collide()

            # 更新窗口显示
            pygame.display.update()

    def __check_collide(self):
        """碰撞检测"""

        # 子弹摧毁敌机
        enemies = pygame.sprite.groupcollide(self.enemy_group,
                                             self.hero.bullets,
                                             False,
                                             True).keys()
        for enemy in enemies:
            enemy.life -= 1

            if enemy.life <= 0:
                enemy.add(self.destroy_group)
                enemy.remove(self.enemy_group)

                enemy.destroied()

        # 敌机撞毁英雄
        for hero in pygame.sprite.spritecollide(self.hero,
                                                self.enemy_group,
                                                True):
            print("英雄牺牲了...")
            self.hero.destroied()

    def __event_handler(self):
        """事件处理"""

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                PlaneGame.__finished_game()
            elif event.type == HERO_FIRE_EVENT:
                self.hero.fire()
            elif event.type == CREATE_ENEMY_EVENT:
                self.enemy_group.add(Enemy())
            # 按下 b 英雄自爆
            elif event.type == pygame.KEYDOWN and event.key == pygame.K_b:
                # self.hero.destroied()

                # 集体自爆
                for enemy in self.enemy_group.sprites():
                    enemy.destroied()

        # 判断英雄是否已经被销毁，如果是，游戏结束！
        if self.hero.can_destroied:
            PlaneGame.__finished_game()

        # 通过 pygame.key 获取用户按键
        keys_pressed = pygame.key.get_pressed()
        dir = keys_pressed[pygame.K_RIGHT] - keys_pressed[pygame.K_LEFT]

        # 根据移动方向设置英雄的速度
        self.hero.speed = dir * 2

    def __update_sprites(self):
        """更新/绘制精灵组"""

        for group in [self.back_group, self.hero_group,
                      self.hero.bullets, self.enemy_group,
                      self.destroy_group]:
            group.update()
            group.draw(self.screen)

    @staticmethod
    def __finished_game():
        """退出游戏"""

        print("退出游戏")
        pygame.quit()
        exit()


if __name__ == '__main__':
    # 创建游戏对象并启动游戏
    PlaneGame().start_game()

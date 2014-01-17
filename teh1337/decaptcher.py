#!/usr/bin/env python
#
# Decaptcher Python API Module
# By Brandon Smith (brandon.smith@studiobebop.net)
#
import requests

class decaptcher:
    def __init__(self, username, password, product_id=""):
        self.__action_url = "http://poster.decaptcher.com/"
        self.__username = username
        self.__password = password
        self.__product_id = product_id

    def get_balance(self):
        data = {"function": "balance",
                "username": self.__username,
                "password": self.__password}
        request = requests.post(self.__action_url, data)
        return float(request.content)

    def solve_image(self, image_path):
        data = {"function": "picture2",
                "username": self.__username,
                "password": self.__password,
                "pict_to": "0",
                "pict_type": "0",
                "pict": open(image_path, "rb").read()}
        request = requests.post(self.__action_url, data)
        answer = request.content.split("|")[-1]
        return answer
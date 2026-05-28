// MagicPodsLinux: https://github.com/steam3d/MagicPodsLinux
// Copyright: 2020-2026 Aleksandr Maslov <https://magicpods.app>
// License: GPL-3.0

import QtQuick 2.15

QtObject {
    id: root

    property var animations: ({
        "76_8195": {
            "source": "76_8195.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8197": {
            "source": "76_8197.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8198": {
            "source": "76_8198.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8201": {
            "source": "76_8201.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202": {
            "source": "76_8202.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202_2": {
            "source": "76_8202_2.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202_3": {
            "source": "76_8202_3.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202_15": {
            "source": "76_8202_15.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202_49": {
            "source": "76_8202_49.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8202_17": {
            "source": "76_8202_17.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8203": {
            "source": "76_8203.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8203_0": {
            "source": "76_8203_0.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8204": {
            "source": "76_8204.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8206": {
            "source": "76_8206.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8207": {
            "source": "76_8207.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8224": {
            "source": "76_8224.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8208": {
            "source": "76_8208.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8209": {
            "source": "76_8209.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8209_34": {
            "source": "76_8209_34.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8209_102": {
            "source": "76_8209_102.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8210": {
            "source": "76_8210.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8210_51": {
            "source": "76_8210_51.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8211": {
            "source": "76_8211.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8212": {
            "source": "76_8212.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8214": {
            "source": "76_8214.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8217": {
            "source": "76_8217.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8219": {
            "source": "76_8219.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8221": {
            "source": "76_8221.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8221_2": {
            "source": "76_8221_2.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8223": {
            "source": "76_8223.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8223_18": {
            "source": "76_8223_18.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8223_84": {
            "source": "76_8223_84.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8223_86": {
            "source": "76_8223_86.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8228": {
            "source": "76_8228.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8231": {
            "source": "76_8231.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8237_18": {
            "source": "76_8237_18.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8237": {
            "source": "76_8237.png",
            "frameWidth": 200,
            "frameHeight": 132,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8239": {
            "source": "76_8239.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        },
        "76_8239_187": {
            "source": "76_8239_187.png",
            "frameWidth": 220,
            "frameHeight": 110,
            "frameCount": 360,
            "frameRate": 60
        }
    })

    property string fallbackKey: "76_8207"

    function get(key) {
        if (animations[key])
            return animations[key];
        return animations[fallbackKey];
    }
}

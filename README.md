## 项目用途
获取必应今日美图，并将其设置为桌面壁纸。

## 原理说明

参考：[必应美图和每日故事 APi 接口的使用方法（说明）](https://blog.gazer.win/essay/the-usage-of-bing-daily-stories-and-daily-wallpaper.html)

接口链接：`https://cn.bing.com/HPImageArchive.aspx?format=xml&cc=jp&idx=0&n=1`

参数说明：

| 参数   | 性质 | 说明                                                         |
| :----- | :--- | :----------------------------------------------------------- |
| format | 可选 | 从字面意思上理解就是格式的意思，取值范围{js、xml}，接口将根据此参数返回对应的 XML 或者 JSON 格式的数据，缺省（或缺失）将默认返回 XML 文档数据格式。 |
| cc     | 可选 | 国家（含地区）代码（Country Code）的英文缩写，表示获取相应国家（或地区）的必应美图（需要国外主机，国内主机请求一律返回中国区的必应美图），目前已知的可取值范围 {ar、at、au、be、br、ca、ch、cl、cn、de、dk、es、fi、fr、hk、ie、in、it、jp、kr、nl、no、nz、ph、pt、ru、se、sg、tw、uk}，对应的地区请对照此列表 → [传送门](https://blog.gazer.win/essay/top-level-geographic-domain-suffix-for-each-country-or-region-in-the-world.html)，缺省（或缺失）将根据请求来源的IP机请求的必应地区返回相应地区的必应美图（划重点，并不是每个地区都有属于自己独一无二的美图，未预设美图的地区将直接引用国际版 Bing 美图。并且在配合国外主机使用此参数时抓取信息时，需要将接口的主机地址“cn.bing.com”替换成上述任意一个地区的必应，例如德国的“de.bing.com”才能获取到相应“cc”表示的地区的美图，否则一律返回中国区的美图信息。 |
| idx    | 可选 | 取值范围[-1,7]之间的整数，分别表示 {明天、今天、昨天、前天、三天前、依此类推……} 的图片，缺省（或缺失）则默认为“0”，表示今天的美图。 |
| n      | 必选 | 缺省（或缺失）将返回 null（空值），导致无法获取美图信息。取值范围是[1,8]之间的整数，表示从“idx”指定的日期开始往前推共“n”张美图信息。 |

## 构建说明
项目依赖 JSON 库[rapidjson](https://github.com/Tencent/rapidjson)

## License
[MIT](https://github.com/songbaoming/BingTodayImage/blob/master/LICENSE)

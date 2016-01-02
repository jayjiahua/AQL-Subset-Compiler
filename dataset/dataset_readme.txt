共有4个dataset

其中Perloc和Revenue是原例子, 用于测试结果.

另外两个分别为NBA和Image

====== NBA ======
1. 文章来源:
  NBA.txt ------ http://www.nba.com/2015/news/12/25/kobe-bryant-leads-in-first-returns-of-nba-all-star-voting-2016/index.html
  NBA2.txt ----- http://www.nba.com/2015/news/features/david_aldridge/12/28/morning-tip-top-10-stories-from-2015-nba-virtual-reality-memphis-grizzlies-changes/index.html

2. 背景
  选取的文章主要是关于NBA本赛季各个明星球员和队伍的情况,以及2016年NBA全明星周末的各个明星球员的得票情况.
  AQL语句的目的是提取各个明星球员和队伍的详细信息.根据观察,明星球员的名字和球队的名字一般又若干个以大写字母开头的单词组合而成,而且后面通常紧跟着一对括号,括号内有关于该球员或球队的补充信息(比如属于哪支球队,场均数据,得票数等等).

3. AQL语句解释
  第一句: 根据正则表达式, 匹配开头字母是大写,其他字母为小写的单词, 创建视图BigWord
  第二句: 根据正则表达式, 匹配圆括号及括号里面的内容, 创建视图Data
  第三句: 匹配1~3个BigWord和Data的组合,即获得球员名字及其得票数, 创建视图NameAndData
  第四句: 队名出现的地方格式为: "队名' 队员", 通过已有的列和正则组合成pattern进行匹配, 创建视图TeamAndPlayer
  第五句和第六句: 分别提取NameAndData和TeamAndPlayer的某一列, 创建两个视图
  然后依次输出6个视图

====== Image ======
1. 文章来源
  image1.txt ------ http://www.datasciencecentral.com/profiles/blogs/interesting-data-science-application-steganography
  image2.txt ------ http://www.rideau-info.com/photos/whatis.html
  image3.txt ------  http://www.ks.uiuc.edu/Research/vmd/vmd-1.7.1/ug/node45.html

2. 背景
  选取的文章主要是关于图像处理这一方面的, 里面有一些颜色值和分辨率的数值.

3. AQL语句解释
  第一句: 使用正则匹配数字, 创建试图Number
  第二句: 使用正则匹配R,G,B字符,创建视图Color
  第三句: 使用Color=Number的pattern, 匹配形如R=100这样的颜色值, 创建视图RGBunit
  第四句: 使用三个连续的RGBunit列的pattern, 匹配形如R=100,G=200,B=180这样的字符串, 创建视图RGB
  第五句: 匹配分辨率, 形如 1000 * 1000的字符串, 创建视图Pixel
  第六句: 从RGB中选取其中两列创建视图
  然后依次输出6个视图

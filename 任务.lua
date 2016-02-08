--[[

外挂任务脚本



脚本分几个类型, 通过配置文件指定, 外挂解析配置文件为每种类型的脚本做不同的处理.

任务脚本的处理:
	从脚本获取能处理的任务();
	while(true)
	{
       call 获取屏幕
       if(匹配某个任务){
	        call 这个任务
	   }
	}

日常脚本的处理方式:
  
	if 获取当前日常次数 < 30
		call 日常脚本
	else
		完成脚本   (30次做完了)


所有可用的函数, 暂时这么多. 有需要再加

"点击座标(x, y)"    	--例如  点击座标(0, 0)   会点击游戏窗口左上角 窗口600x460的话, 点击座标(600, 460) 就是右下角
"获取玩家状态"      	--这个函数应该不会用到, 外挂应该尽可能封装底层的东西, 比如玩家状态是战斗中会需要的函数封装出来
"当前城市"      		--这个函数其实也是用来给脚本写逻辑的, 应该也不会用到
"对话"				--和npc对话, npc又都在屏幕中间(自动寻路的结果), 其实就是点击屏幕中间 点击座标(600/2, 460/2)
"点击对话框内图片"  	--和下面函数类似, 只是会先给游戏窗口获得焦点,   梦幻中的对话框只能在焦点状态下点击才会有效(这是游戏的行为)
"点击图片"          	--点击屏幕上匹配的地方     比如 点击图片("夏大叔")
"等待停止奔跑"   	--一般跟着上面的函数写, 表示点击一个图片后, 玩家开始移动了, 此时开始等待停止
"装备物品"     		--装备背包内某个物品    比如  装备物品("锤子手机");
"点击小地图"    		--有个任务是点开小地图, 找到某个位置,  这个函数应该很少用
"右击"				--右击("厦大叔")    右键点击某个图片
"加血" 				--右击点击玩家血条
"使用辅助技能"        --使用辅助技能("烹饪")     实际操作:玩家头像->辅助技能->烹饪
"存在图片()" 		--返回真假值   存在图片("厦大叔") 用来判断厦大叔.png是否在游戏内匹配到 战斗脚本里有用到
"玩家宠物()"			--玩家宠物("参战")     点击玩家宠物面板的参战按钮
"结束脚本()"			--脚本正常的退出执行, 报告控制台, 控制台决定下一步的操作, 可以带参数表示原因 比如 结束脚本("中了500万, 先停停..")
"抓鬼()"			--当前地图内随机移动, 直到进入战斗
"释放法术()"			--战斗状态时释放一个法术 比如 释放法术("大爷的")
"点击任务()"			--从任务区域匹配, 能增加匹配速度, 还能减少误匹配

]]

-- 声明这个脚本能处理的任务
任务={
"霞姑娘",
"刘大婶",
"孙猎户",
"召唤兽参战",
"道具",
"干掉野猪",
"孙厨娘",
"红色任务血条",
"烹饪包子",
"郭大哥",
"赶走捣乱的狸",
"玄大夫",
"雨画师什么事",
"地图找平儿",
"这就上船",
"告别夏大叔"
}; 


function 霞姑娘()
   点击任务("霞姑娘");
   对话(); 
end

function 刘大婶()
   点击任务("刘大婶");
   对话(); 
end

function 孙猎户()
   点击任务("孙猎户");
   对话(); 
end

function 道具()
   装备物品("青铜短剑");
   装备物品("折扇");
   装备物品("布衣");
   装备物品("布裙");
   装备物品("双短剑");
   装备物品("牛皮鞭");
   装备物品("曲柳仗");
   装备物品("黄铜圈");
end

function 干掉野猪()
	点击任务("干掉野猪");
	点击座标(269,253);
	点击对话框内图片("对话框-战斗");
end

function 孙厨娘()
   点击任务("孙厨娘");
   对话(); 
end

function 红色任务血条()
   加血();
   对话(); 
end

function 烹饪包子()
   使用辅助技能("烹饪");
end

function 郭大哥()
   点击任务("郭大哥");
	对话();
end

function 赶走捣乱的狸()
   点击任务("赶走捣乱的狸");
	对话();
    点击对话框内图片("对话框-战斗");
end

function 召唤兽参战()
   玩家宠物("参战");
end

function 玄大夫()
   点击任务("玄大夫");
   对话();
   点击对话框内图片("玄大夫治病");
end


function 雨画师什么事()
   点击任务("雨画师什么事");
	对话();
end

function 地图找平儿()
   点击小地图("去找萍儿");
   对话();
end

function 这就上船()
   点击任务("这就上船");
   对话();
   点击对话框内图片("这就上船啦");
end

function 告别夏大叔()
   点击任务("告别夏大叔");
   对话();
end




-- 处理战斗场景
-- 外挂检测当前游戏场景, 如果战斗场景, 回调此脚本
-- 外挂回调前检测 法术-防御 这个菜单是否出现
-- 如果 菜单出现
-- 调用 战斗回调()
-- 如果 战斗回调返回 true 这个处理的过程就完了
-- 如果 战斗回调返回 false 外挂默认去点击一次 "自动战斗"
function 战斗回调()  --这个名字固定了, 外挂会指定调用这个名字, 


 	--这里的逻辑一般是这样:
	if(存在图片("狸")) then    --语法 if then  end
		--处理怪物是狐狸的情况
        点击图片("捕捉");
		点击图片("狸");
                       
	end   


	--检测朝向看能不能这样
	--外挂匹配图片并不是100%, 这个相似度可以自定义
	--外挂又能匹配某片区域, 这个有能避免匹配到无关的东西
	--如果战斗场景下, 敌方区域在屏幕上的显示是   x 30, y30  宽度100, 高度100的话
	--匹配这片区域, 同时相似度50% 就能匹配到了. 匹配到就能获取这个怪的屏幕坐标, 用法术封锁什么的应该可以
	
 	
	return true
end   --这个end意味函数结束   类似 c中的  {}
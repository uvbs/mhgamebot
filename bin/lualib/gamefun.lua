



function 检测需要人工()

	--人工帮助
	if 存在任务("需要人工标志1") then
		发送人工请求()
		return
	end

	--人工帮助
	if 存在任务("需要人工标志2") then
		发送人工请求()
		return
	end

	--人工帮助
	if 存在任务("需要人工标志3") then
		发送人工请求()
		return
	end

	--人工帮助
	if 存在任务("需要人工标志4") then
		发送人工请求()
		return
	end

end


function 检查装备耐久()
	按键("ALT+E")
	if 存在图片("没有耐久") then

	end
		
end


function 玩家宠物(名字)
	点击坐标(434, 21)
	点击图片(名字)
end

function 当前城市(城市名)
	local map = "map/"..城市名
	if 存在图片(map) then
		return true
	end
	
	return false
end


function 移动直到遇到(x, y, npc, 匹配程度)

	while true do
		if 存在图片(npc) then
			return
		else
			点击坐标(x, y)
			等待停止奔跑()
		end

		延迟(1)
	end
	
end


function 走到旁边(npc, thershold)
	thershold = thershold or 0
	if thershold == 0 then
		thershold = 默认程度
	end

	local x,y = 获得图片位置(npc, thershold)  --默认7的话偶尔匹配不到

	--调整到旁边
	if x < 45 then 
		x = x + 45		--40容易挡着目标
	else
		x = x - 45
	end

	点击坐标(x, y)
	等待停止奔跑()
end
	
function 点击小地图坐标(x, y)

	--游戏内位置
	local x, y = 获得图片位置("关闭")
	x = x - 5;
	y = y - 5;

	local mouseX, mouseY;
	--每个地区比例不同, 比例数据需要一个一个找出来
	if 当前城市("长安城") then
		mouseX = 地图比例["长安城x"] * x 
		mouseY = 地图比例["长安城y"] * y
	else
		调试信息("城市地图比例未知")
	end
	

	点击坐标(x, y)
end


function 使用辅助技能(名字)
    if 名字 == "烹饪" then
		点击坐标(548, 27)  --玩家头像
        点击图片("辅助技能")
        点击图片(名字)
    else
        调试信息("参数错误 找唱哥")
	end
	
end


function 确保接触到NPC()
	返回 = false
	--判断是否接触NPC
	if 存在图片("接触NPC标识1", 0.95) then
		返回 = true
	elseif 存在图片("接触NPC标识2", 0.95) then
		返回 = true
	elseif 存在图片("接触NPC标识3", 0.95) then
		返回 = true
	elseif 存在图片("接触NPC标识4", 0.95) then
		返回 = true
	else
		error("没有接触到NPC!")
	end

	if 返回 then
		调试信息("已接触到NPC")
	end

	return 返回
end


function 等待标识消失()
	--等待一些标识消失, 避免遮挡
	等待消失("信息提示标志")
end


--原本封装的移动到 gamelib 中
function 对话(x, y, nowait)

	nowait = nowait or 0
	等待标识消失()
	关闭无关窗口()
	
	隐藏玩家()


	ox = 640/2
	oy = 480/2

	y = y or 0
	x = x or 0

	ox = ox + x
	oy = oy + y

	oy = oy - 20  --稍微上面一点

	移动鼠标到(ox, oy)

	--判断是否接触NPC
	if 确保接触到NPC() then
		点击()
	end
	
	if nowait == 0 then
		等待对话框出现()
	end
end


function 关闭无关窗口()
	调试信息2("关闭无关窗口")
	for i=3,1,-1 do 
		if 存在图片("关闭") then	--0.8的时候, 试验消耗的提示下不会匹配到
			点击图片("关闭")
		elseif 存在图片("关闭1") then
			点击图片("关闭1")
		elseif 存在图片("关闭2") then
			点击图片("关闭2")
		elseif 存在图片("取消") then
			点击图片("取消")
		elseif 存在图片("世界按钮", 0.95) then --小地图存在
			按键("TAB")
		elseif 存在图片("传送地图标志", 0.95) then
			右击("传送地图标志")
		end
	end
end


function 推荐加点()
	按键("ALT+W")
	点击图片("推荐加点按钮")
	点击图片("加点试试按钮")
	延迟(5)
	点击图片("确认加点按钮")
end

function 打开任务窗口()
	调试信息("打开任务窗口")
	if 存在图片("忽略任务提示按钮") then
		调试信息("任务窗口已经打开")
	else
		按键("ALT+Q")
		等待出现("忽略任务提示按钮")
	end
end


function 等待出现(名字, 程度)
	调试信息("等待出现["..名字.."]")

	程度 = 程度 or 0
	if 程度 == 0 then
		程度 = 0.9
	end

	没有出现 = true
	for i=15,1,-1  do 
		if 存在图片(名字, 程度) then 
			没有出现 = false
			break
		end

		延迟(1)
	end

	if 没有出现 then
		error("等待"..名字.."出现失败")
		return
	end
end

function 等待消失(名字)
	没有消失 = true
	for i=15,1,-1  do 
		if 存在图片(名字, 0.9) == false then 
			没有消失 = false
			break
		end

		延迟(1)
	end

	if 没有消失 then
		error("等待消失失败"..名字)
	end
end


function 领取日常任务(任务名)
	调试信息("领取日常任务"..任务名)
	点击坐标(130, 60)	--点击日历
	点击图片("日常任务按钮")
	点击图片("活动日历标题", -205, 63)

	等待标识消失()

	if 任务名 == "王夫人" then
		点击图片("日常/王夫人任务", 86, 31)
		点击对话框("正闲着呢")
	elseif 任务名 == "除暴安良" then
		点击图片("日常/除暴安良", 86, 31) --领取
		点击对话框("我要助你除暴安良")
	elseif 任务名 == "初入江湖" then
		点击图片("日常/初入江湖", 86, 31) --领取
		点击对话框("我要助你除暴安良")
	else
		error("暂时还没写其他日常")
	end

	--点击对话框("等待关闭")
end


function 释放法术(法术名)
	if 当前玩家状态() == 战斗状态 then
		error("非战斗状态使用了法术, 检查脚本逻辑")
	end

	if 存在图片("战斗-法术") then
		点击图片("战斗-法术")
	elseif 存在图片("战斗-法术1") then
		点击图片("战斗-法术1")
	end

	点击图片(法术名)

end

function 处理死亡()
	if 当前城市("轮回司") then
		调试信息("处理死亡")
		点击图片("轮回司标识", -70, 0)
		等待停止奔跑()
		点击图片("轮回司标识", -150, 20)
		点击对话框("好啊")
	end
end


function 前往(城市名)
	if 城市名 == "小地图-镖" then
	
	elseif 城市名 == "朱紫国" then
		if 当前城市("长安城") then
			装备物品("飞行符", 1)
			点击图片("小地图/朱紫国")
		end
	elseif 城市名 == "建邺城" then
		if 当前城市("长安城") then
			装备物品("飞行符", 1)
			点击图片("小地图/建邺城")
		end
	elseif 城市名 == "西梁女国" then
		if 当前城市("长安城") then
			装备物品("飞行符", 1)
			点击图片("小地图/小地图-西梁女国")
		end
	elseif 城市名 == "东海湾" then

		if 当前城市("建邺城") then
			前往("小地图-东海湾")
		else
			装备物品("飞行符", 1)
			点击图片("小地图/建邺城")	
		end
	elseif 城市名 == "傲来国" then
		装备物品("飞行符", 1)
		点击图片("小地图/傲来国")
	elseif 城市名 == "长寿村" then
		装备物品("飞行符", 1)
		点击图片("小地图/长寿村")
	elseif 城市名 == "宝象国" then
		if 当前城市("长安城") then
			装备物品("飞行符")
			点击图片("小地图/宝象国")
		end
	--上面都是城市间飞行

	elseif 城市名 == "小地图-龙宫" then
		点击小地图(110, 95)
	elseif 城市名 == "小地图-龙宫-屋子" then
		点击小地图(111, 71)
		点击坐标(437, 148)
	elseif 城市名 == "小地图-长寿村" then
		点击小地图(城市名, 50, 0)
		点击坐标(640/2, 30)
	elseif 城市名 == "花果山" then
		if 当前城市("长安城") then
			装备物品("飞行符", 1)
			点击图片("小地图/傲来国")
		elseif 当前城市("傲来国") then
			点击小地图("花果山", 20, 0)
			点击坐标(588, 129)
		end
	elseif 城市名 == "小地图-化生寺-屋子" then
		点击小地图(90, 53)
		点击坐标(441, 162)
	elseif 城市名 == "小地图-长寿郊外" then

		if 当前城市("长安城") then
			装备物品("飞行符", 1)
			点击图片("小地图/长寿村")
		elseif 当前城市("长寿村") then
			点击小地图(142, 5)
			点击坐标(570, 440)			
		else
			点击小地图(48, 9);
			隐藏玩家()
			点击图片("跑镖-传送长寿郊外NPC")
			点击对话框("对话框-是的我要去")
		end
		
	elseif 城市名 == "小地图-酒" then

	elseif 城市名 == "小地图-镇元大仙-屋子" then
		点击小地图(55,35)
		点击坐标(510, 94);
	elseif 城市名 == "小地图-观音洞" then

	elseif 城市名 == "小地图-赌" then
		点击小地图(283,41)
	elseif 城市名 == "小地图-秦琼" then
		点击小地图(87, 76);
		if 当前城市("长安城") then 
			点击坐标(200, 200) 
		end
	elseif 城市名 == "小地图-程咬金-屋子" then
		点击小地图(78, 46)
		点击坐标(200, 200)  --进屋子
	elseif 城市名 == "小地图-盘丝岭" then
		点击小地图(530,111)
		点击坐标(640 / 2, 50)
	elseif 城市名 == "小地图-狮坨岭" then
		点击小地图(15, 50)
		点击坐标(20, 480/2)
	elseif 城市名 == "小地图-盘丝岭-屋子" then
		点击小地图(188, 128);
		点击坐标(510, 94);
	elseif 城市名 == "小地图-潮音洞" then
		点击小地图(10, 60);
		点击坐标(100, 100)  --传送门
	elseif 城市名 == "小地图-江南野外" then
		点击小地图(535,8)
		点击坐标(600, 420)
	elseif 城市名 == "小地图-普陀山" then
		点击小地图(228, 65);

		if 存在图片("跑镖-普陀接引仙女") == false then
			点击坐标(640/2-60, 480/2)
			等待停止奔跑()
		end
		隐藏玩家()
		点击图片("跑镖-普陀接引仙女")
		点击对话框("对话框-是的我要去")
	elseif 城市名 == "小地图-方寸山" then
		点击小地图(城市名, 30, 0)
		点击坐标(640/2, 20)
	elseif 城市名 == "小地图-建邺城" then
		if 当前城市("江南野外") then
			点击小地图(144,55)
			点击坐标(620, 100);
		end
	elseif 城市名 == "小地图-女儿村" then
		点击小地图(城市名, -30, 0)
		点击坐标(100, 100)
	elseif 城市名 == "小地图-女儿村-屋子" then
		点击小地图(城市名)
		点击坐标(100, 480/2)
	elseif 城市名 == "小地图-天宫" then
		点击小地图(25,55)
		按键("F9")
		点击图片("跑镖-天将")
		点击对话框("对话框-是的我要去")
	elseif 城市名 == "小地图-天宫-屋子" then
		点击小地图(151, 60)
		点击坐标(160, 160)  --进传送门

	elseif 城市名 == "小地图-大唐官府" then
		点击小地图(312,272)
		点击坐标(640/2, 40)
	elseif 城市名 == "小地图-大唐境外" then
		点击小地图(7, 78);
		点击坐标(30, 480/2)  --进传送门	
	elseif 城市名 == "小地图-大唐国境" then
		点击小地图(7, 5)
		点击传送门()
	elseif 城市名 == "小地图-地府" then
		点击小地图(49, 331)
		点击坐标(200, 200)  --传送门
	elseif 城市名 == "小地图-地府-屋子" then
		点击小地图(32,66)
		点击坐标(200, 200)
	elseif 城市名 == "小地图-国子监" then
		点击小地图(城市名, 40, -50)
		点击坐标(200, 47) --传送门
	elseif 城市名 == "小地图-化生寺" then
		点击小地图(504, 272)
		点击坐标(510, 94)  --传送门
	elseif 城市名 == "小地图-傲来国" then
		点击小地图(城市名)
		隐藏玩家()
		if(存在图片("驿站老板", 0.5)) then 
			点击图片("驿站老板", 0.5)
		elseif(存在图片("驿站老板1", 0.5)) then  
			点击图片("驿站老板1", 0.5) 
		elseif(存在图片("跑镖-驿站老板2", 0.5)) then  
			点击图片("跑镖-驿站老板2", 0.5)  
		elseif 存在图片("跑镖-驿站老板3", 0.5) then
			点击图片("跑镖-驿站老板3", 0.5)
		else
			return
		end

		点击对话框("对话框-是的我要去")

	elseif 城市名 == "小地图-五庄观" then
		点击小地图(630, 74)
		点击坐标(510, 94);
	elseif 城市名 == "小地图-中间" then


	elseif 城市名 == "小地图-二大王" then
		点击小地图(27, 82);  --点的是小地图位置
		点击坐标(640/2, 100)

	elseif 城市名 == "小地图-三大王" then
		点击小地图(15, 41);
		点击坐标(640/2 + 50, 100)

	elseif 城市名 == "小地图-大大王" then
		点击小地图(115, 25);  --点的是小地图位置
		点击坐标(510, 94);      --进传送门



	elseif 城市名 == "小地图-东海湾" then
		点击小地图(270,30)
		点击坐标(510, 350);



	elseif 城市名 == "小地图-菩提祖师-屋子" then
		点击小地图(城市名)
		点击坐标(500, 100)
	elseif 城市名 == "小地图-魔王寨" then
		点击小地图(55, 114)
		点击坐标(640/2, 20)
	elseif 城市名 == "小地图-魔王寨-屋子" then
		点击小地图(94, 73)
		点击坐标(640/2 + 50, 480/2 - 50);
	else
		error("前往 错误的地图")
	end
end


function 隐藏玩家()
	按键("F9")
	按键("ALT+H")
end


function 找驿站老板()

	::重试::
	
	if(当前城市("长安城")) then
		前往("小地图-赌");
		隐藏玩家()

		if(存在图片("驿站老板", 0.5)) then 
			点击图片("驿站老板", 0.5)

		elseif(存在图片("驿站老板1", 0.5)) then
			点击图片("驿站老板1", 0.5)

		elseif(存在图片("跑镖-驿站老板2", 0.5)) then 
			点击图片("跑镖-驿站老板2", 0.5)

		elseif(存在图片("跑镖-驿站老板3", 0.5)) then 
			点击图片("跑镖-驿站老板3", 0.5) 
		else
			调试信息("竟然没有找到...重试")
			goto 重试
		end

		等待对话框出现()
		if(存在图片("对话框/对话框-是的我要去")) then
			点击对话框("对话框-是的我要去");
		else
			goto 重试
		end
		
	end
end
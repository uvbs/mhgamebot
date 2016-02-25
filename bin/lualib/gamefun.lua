

--地图比例 = 窗口宽度 / 游戏内宽度
--窗口宽度 = 游戏内宽度 * 比例
地图比例 = {}
地图比例["长安城x"] = 371/95
地图比例["长安城y"] = 1



function 检查装备耐久()
	按键("ALT+E")
	if 存在图片("没有耐久") then

	end
		
end

--血量少于一定值吃包子
function 检查血量()
	if 存在图片("血量-满") then
		调试信息("血值还满")
	elseif 存在图片("血量-较多") then
		调试信息("血值还可以")		
	elseif 存在图片("血量-中") then
		调试信息("血值有点少, 吃药")
		装备物品("包子")	
	elseif 存在图片("血量-危险") then
		调试信息("血值能到这份还跑脚本也算人才")
		装备物品("包子")
	end
end

function 玩家宠物(名字)
	变动点击(434, 21)
	点击图片(名字)
end

function 当前城市(城市名)
	local map = "map\\"..城市名
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

function 走到旁边(npc)
	local x,y = 获得图片位置(npc)

	--调整到旁边
	if x < 40 then 
		x = x + 40
	else
		x = x - 40
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
		变动点击(548, 27)  --玩家头像
        点击图片("辅助技能")
        点击图片(名字)
		关闭无关窗口()
    else
        调试信息("参数错误 找唱哥")
	end
	
end
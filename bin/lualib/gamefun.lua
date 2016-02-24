

--地图比例 = 窗口宽度 / 游戏内宽度
--窗口宽度 = 游戏内宽度 * 比例
地图比例 = {}
地图比例["长安城x"] = 371/95
地图比例["长安城y"] = 1

function 对话()
	--点击屏幕中间
	点击坐标(640/2, 480/2)

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

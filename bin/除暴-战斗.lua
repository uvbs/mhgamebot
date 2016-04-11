-- 处理战斗场景

function 战斗回调()  --这个名字固定了, 外挂会指定调用这个名字, 

	if 存在图片("自动战斗中") then
		
	elseif 存在图片("自动战斗") then	
		点击图片("自动战斗")
	end	
	return false		--外挂会执行默认的动作 -自动战斗
end

function 宠物战斗回调()
	按键("ALT+A")
end


--��ͼ���� = ���ڿ�� / ��Ϸ�ڿ��
--���ڿ�� = ��Ϸ�ڿ�� * ����
��ͼ���� = {}
��ͼ����["������x"] = 371/95
��ͼ����["������y"] = 1

function �Ի�()
	--�����Ļ�м�
	�������(640/2, 480/2)

end


function ��ǰ����(������)
	local map = "map\\"..������
	if ����ͼƬ(map) then
		return true
	end
	
	return false
end


function �ƶ�ֱ������(x, y, npc, ƥ��̶�)

	while true do
		if ����ͼƬ(npc) then
			return
		else
			�������(x, y)
			�ȴ�ֹͣ����()
		end

		�ӳ�(1)
	end
	
end

function �ߵ��Ա�(npc)
	local x,y = ���ͼƬλ��(npc)

	--�������Ա�
	if x < 40 then 
		x = x + 40
	else
		x = x - 40
	end

	�������(x, y)
	�ȴ�ֹͣ����()
end
	
function ���С��ͼ����(x, y)

		--��Ϸ��λ��
		local x, y = ���ͼƬλ��("�ر�")
		x = x - 5;
		y = y - 5;

		local mouseX, mouseY;
		--ÿ������������ͬ, ����������Ҫһ��һ���ҳ���
		if ��ǰ����("������") then
			mouseX = ��ͼ����["������x"] * x 
			mouseY = ��ͼ����["������y"] * y
		else
			������Ϣ("���е�ͼ����δ֪")
		end
		

		�������(x, y)
end

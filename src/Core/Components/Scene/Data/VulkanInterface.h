#pragma once
class VulkanInterface
{
public:
	virtual void CreateVulkanStuff() = 0;
	virtual void DestroyVulkanStuff() = 0;
};

/*
MIT License

Copyright (c) 2019 CSC Project

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <vulkan/vulkan.h>
#include <string.h>
#include <stdio.h>

#define CSC_VK_LAYER_COUNT 32
int csc_vk_layer_exist (char const * layername)
{
	uint32_t count = CSC_VK_LAYER_COUNT;
	VkLayerProperties available [CSC_VK_LAYER_COUNT];
	vkEnumerateInstanceLayerProperties (&count, available);
	for (uint32_t i = 0; i < count; i ++)
	{
		//TRACE_F ("%s %s", available [i].layerName, available [i].description);
		int diff = strcmp (layername, available [i].layerName);
		if (diff == 0) {return 1;}
	}
	return 0;
}

int csc_vk_layers_exist (char const * layername [], int n)
{
	for (int i = 0; i < n; i ++)
	{
		if (csc_vk_layer_exist (layername [i]) == 0)
		{
			return 0;
		}
	}
	return 1;
}


void csc_vk_pinfo (void const * item)
{
	struct csc_vk_struct
	{
		VkStructureType sType;
	};
	struct csc_vk_struct const * p = (struct csc_vk_struct *) item;
	switch (p->sType)
	{
	case VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO:{
		VkInstanceCreateInfo const * info = (VkInstanceCreateInfo const *)item;
		for (uint32_t i = 0; i < info->enabledExtensionCount; ++i)
		{
			printf ("ppEnabledLayerNames %s\n", info->ppEnabledExtensionNames [i]);
		}
		break;}
	}
}

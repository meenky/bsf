/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2011 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#include "CmD3D9VertexDeclaration.h"
#include "CmD3D9Mappings.h"
#include "CmException.h"
#include "CmD3D9RenderSystem.h"
#include "CmD3D9ResourceManager.h"

namespace BansheeEngine 
{
	D3D9VertexDeclaration::D3D9VertexDeclaration(const VertexDeclaration::VertexElementList& elements)
		:VertexDeclaration(elements)
    { }

    D3D9VertexDeclaration::~D3D9VertexDeclaration()
    {    }

	void D3D9VertexDeclaration::destroy_internal()
	{
		releaseDeclaration();

		VertexDeclaration::destroy_internal();
	}

	void D3D9VertexDeclaration::notifyOnDeviceCreate(IDirect3DDevice9* d3d9Device)
	{
		
	}

	void D3D9VertexDeclaration::notifyOnDeviceDestroy(IDirect3DDevice9* d3d9Device)
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToDeclarationIterator it = mMapDeviceToDeclaration.find(d3d9Device);

		if (it != mMapDeviceToDeclaration.end())	
		{
			SAFE_RELEASE(it->second);	
			mMapDeviceToDeclaration.erase(it);
		}
	}

    IDirect3DVertexDeclaration9* D3D9VertexDeclaration::getD3DVertexDeclaration()
    {
		IDirect3DDevice9* pCurDevice   = D3D9RenderSystem::getActiveD3D9Device();
		DeviceToDeclarationIterator it = mMapDeviceToDeclaration.find(pCurDevice);
		IDirect3DVertexDeclaration9* lpVertDecl = NULL;

		// Case we have to create the declaration for this device.
		if (it == mMapDeviceToDeclaration.end() || it->second == NULL)
		{
			D3DVERTEXELEMENT9* d3delems = bs_newN<D3DVERTEXELEMENT9, PoolAlloc>((UINT32)(mElementList.size() + 1));

			VertexElementList::const_iterator i, iend;
			unsigned int idx;
			iend = mElementList.end();
			for (idx = 0, i = mElementList.begin(); i != iend; ++i, ++idx)
			{
				d3delems[idx].Method = D3DDECLMETHOD_DEFAULT;
				d3delems[idx].Offset = static_cast<WORD>(i->getOffset());
				d3delems[idx].Stream = i->getStreamIdx();
				d3delems[idx].Type = D3D9Mappings::get(i->getType());
				d3delems[idx].Usage = D3D9Mappings::get(i->getSemantic());

				if (i->getSemantic() == VES_COLOR)
				{
					d3delems[idx].UsageIndex = 0;
				}
				else
				{
					d3delems[idx].UsageIndex = static_cast<BYTE>(i->getSemanticIdx());
				}
			}
			// Add terminator
			d3delems[idx].Stream = 0xff;
			d3delems[idx].Offset = 0;
			d3delems[idx].Type = D3DDECLTYPE_UNUSED;
			d3delems[idx].Method = 0;
			d3delems[idx].Usage = 0;
			d3delems[idx].UsageIndex = 0;

			
			HRESULT hr = pCurDevice->CreateVertexDeclaration(d3delems, &lpVertDecl);

			if (FAILED(hr))
			{
				BS_EXCEPT(InternalErrorException, "Cannot create D3D9 vertex declaration: ");
			}

			bs_deleteN<PoolAlloc>(d3delems, (UINT32)(mElementList.size() + 1));

			mMapDeviceToDeclaration[pCurDevice] = lpVertDecl;
		}

		// Declaration already exits.
		else
		{
			lpVertDecl = mMapDeviceToDeclaration[pCurDevice];
		}
		
        return lpVertDecl;
    }

	void D3D9VertexDeclaration::releaseDeclaration()
	{
		D3D9_DEVICE_ACCESS_CRITICAL_SECTION

		DeviceToDeclarationIterator it = mMapDeviceToDeclaration.begin();

		while (it != mMapDeviceToDeclaration.end())
		{
			SAFE_RELEASE(it->second);
			++it;
		}	
		mMapDeviceToDeclaration.clear();
	}
}

#include "DXTextureManager.hpp"


void DXTextureManager::SetDevice(ID3D11Device* _device, ID3D11DeviceContext* _context)
{
	m_pd3dDevice = _device;
	m_pImmediateContext = _context;
}

bool DXTextureManager::Load(std::wstring _filename)
{
	auto it = m_TextureList.find(_filename);
	if (it != m_TextureList.end())
	{
		return true;
	}

	DXTexture* newTexture = new DXTexture;
	newTexture->SetDevice(m_pd3dDevice, m_pImmediateContext);
	HRESULT rst = newTexture->Load(_filename);
	if (FAILED(rst))
	{
		delete newTexture;
		OutputDebugStringA("DXTextureManager::Failed Load File.\n");
		return false;
	}

	m_TextureList.insert(std::make_pair(_filename, newTexture));

	return true;
}

bool DXTextureManager::Load(std::string _filename)
{
	std::wstring filename;
	filename.assign(_filename.begin(), _filename.end());

	auto it = m_TextureList.find(filename);
	if (it != m_TextureList.end())
	{
		return true;
	}

	DXTexture* newTexture = new DXTexture;
	newTexture->SetDevice(m_pd3dDevice, m_pImmediateContext);
	HRESULT rst = newTexture->Load(filename);
	if (FAILED(rst))
	{
		delete newTexture;
		OutputDebugStringA("DXTextureManager::Failed Load File.\n");
		return false;
	}

	m_TextureList.insert(std::make_pair(filename, newTexture));

	return true;
}

bool DXTextureManager::Load(const char* _filename)
{
	std::wstring filename(_filename, _filename + std::strlen(_filename));

	auto it = m_TextureList.find(filename);
	if (it != m_TextureList.end())
	{
		return true;
	}

	DXTexture* newTexture = new DXTexture;
	newTexture->SetDevice(m_pd3dDevice, m_pImmediateContext);
	HRESULT rst = newTexture->Load(filename);
	if (FAILED(rst))
	{
		delete newTexture;
		OutputDebugStringA("DXTextureManager::Failed Load File.\n");
		return false;
	}

	m_TextureList.insert(std::make_pair(filename, newTexture));

	return true;
}

DXTexture* DXTextureManager::GetTexture(std::wstring _filename)
{
	auto it = m_TextureList.find(_filename);
	if (it != m_TextureList.end())
	{
		return it->second;
	}
	return nullptr;
}

DXTexture* DXTextureManager::GetTexture(std::string _filename)
{
	std::wstring filename;
	filename.assign(_filename.begin(), _filename.end());
	
	auto it = m_TextureList.find(filename);
	if (it != m_TextureList.end())
	{
		return it->second;
	}
	return nullptr;
}

DXTexture* DXTextureManager::GetTexture(const char* _filename)
{
	std::wstring filename(_filename, _filename + std::strlen(_filename));

	auto it = m_TextureList.find(filename);
	if (it != m_TextureList.end())
	{
		return it->second;
	}
	return nullptr;
}

DXTexture* DXTextureManager::CreateAlphaTexture(int width, int height)
{
	DXTexture* pDXTexture = new DXTexture;
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	float* alphaData = new float[width * height * 4];
	for (size_t row = 0; row < height; row++)
	{
		for (size_t col = 0; col < width; col++)
		{
			float* pixel = &alphaData[width * row * 4 + col * 4];
			pixel[0] = 0.0f;
			pixel[1] = 0.0f;
			pixel[2] = 0.0f;
			pixel[3] = 0.0f;
		}
	}

	ID3D11Texture2D* pTexture;
	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = alphaData;
	initData.SysMemPitch = sizeof(float) * 4 * width;
	initData.SysMemSlicePitch = width * height * 4; //0;
	HRESULT result = m_pd3dDevice->CreateTexture2D(&desc, &initData, &pTexture);
	if (FAILED(result))
	{
		delete pDXTexture;
		return nullptr;
	}

	ID3D11UnorderedAccessView* pUAV;
	D3D11_UNORDERED_ACCESS_VIEW_DESC viewDescUAV;
	ZeroMemory(&viewDescUAV, sizeof(viewDescUAV));
	viewDescUAV.Format = desc.Format;
	viewDescUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	viewDescUAV.Texture2D.MipSlice = 0;
	result = m_pd3dDevice->CreateUnorderedAccessView(pTexture, &viewDescUAV, &pUAV);
	if (FAILED(result))
	{
		return nullptr;
	}

	ID3D11ShaderResourceView* pResourceView;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	result = m_pd3dDevice->CreateShaderResourceView(pTexture, NULL, &pResourceView);
	if (FAILED(result))
	{
		return nullptr;
	}

	pDXTexture->SetTexture2D(pTexture);
	pDXTexture->SetSRV(pResourceView);
	pDXTexture->SetUAV(pUAV);
	m_AlphaTextureList.push_back(pDXTexture);

	return pDXTexture;
}

DXTexture* DXTextureManager::CreateCopyTexture(DXTexture* texture)
{
	if (texture == nullptr)
	{
		return nullptr;
	}

	D3D11_TEXTURE2D_DESC desc;
	texture->GetTexture2D()->GetDesc(&desc);

	ID3D11Texture2D* pTexture;
	HRESULT result = m_pd3dDevice->CreateTexture2D(&desc, NULL, &pTexture);
	if (FAILED(result))
	{
		return nullptr;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	texture->GetResourceView()->GetDesc(&srvDesc);
	ID3D11ShaderResourceView* pSRV;
	result = m_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
	if (FAILED(result))
	{
		return nullptr;
	}

	DXTexture* pDXTexture = new DXTexture;
	pDXTexture->SetTexture2D(pTexture);
	pDXTexture->SetSRV(pSRV);
	CopyTextureList.push_back(pDXTexture);

	return pDXTexture;
}

ID3D11ShaderResourceView* DXTextureManager::CreateShaderResourceViewBuffer(ID3D11Buffer* buffer)
{
	D3D11_BUFFER_DESC descBuf;
	ZeroMemory(&descBuf, sizeof(descBuf));
	buffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else
	{
		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer	

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
	}

	ID3D11ShaderResourceView* pSRV = nullptr;
	HRESULT result = m_pd3dDevice->CreateShaderResourceView(buffer, &desc, &pSRV);
	if (FAILED(result))
	{
		return nullptr;
	}
	else
	{
		SRVList.push_back(pSRV);
		return pSRV;
	}
}


bool DXTextureManager::Initialize()
{
	return true;
}

bool DXTextureManager::Frame()
{
	return true;
}

bool DXTextureManager::Render()
{
	return true;
}

bool DXTextureManager::Release()
{
	if (!m_TextureList.empty())
	{
		for (auto& it : m_TextureList)
		{
			DXTexture* pTexture = it.second;
			if (pTexture != nullptr)
			{
				pTexture->Release();
				delete pTexture;
			}
		}
		m_TextureList.clear();
	}

	for (auto& it : m_AlphaTextureList)
	{
		DXTexture* pTexture = it;
		if (pTexture != nullptr)
		{
			pTexture->Release();
			delete pTexture;
		}
	}
	m_AlphaTextureList.clear();

	for (auto& it : SRVList)
	{
		ID3D11ShaderResourceView* pSRV = it;
		if (pSRV != nullptr)
		{
			pSRV->Release();
		}
	}
	SRVList.clear();

	for (auto& it : CopyTextureList)
	{
		DXTexture* pTexture = it;
		if (pTexture != nullptr)
		{
			pTexture->Release();
			delete pTexture;
		}
	}
	CopyTextureList.clear();

	return true;
}
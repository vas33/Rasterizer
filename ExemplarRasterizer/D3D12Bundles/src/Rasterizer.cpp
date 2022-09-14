#include "stdafx.h"
#include "Rasterizer.h"


Rasterizer::Rasterizer(ID3D12Device* device,
	UINT width, UINT height,
	DXGI_FORMAT format,
	UINT vbSize, UINT ibSize) :
	m_Device(device),
	m_Format(format),
	m_Width(width),
	m_Height(height),
	m_VBSize(vbSize),
	m_IBSize(ibSize)
{
	BuildResources();
}

void Rasterizer::OnResize(UINT newWidth, UINT newHeight)
{
	if ((m_Width != newWidth) || (m_Height != newHeight))
	{
		m_Width = newWidth;
		m_Height = newHeight;
		
		BuildResources();

		BuildDesrciptorsBase();
	}
}

void Rasterizer::BuildResources()
{
	//Create resource for vb 
	//TODO fix VB resource 
	D3D12_RESOURCE_DESC vbDesrciptor;
	ZeroMemory(&vbDesrciptor, sizeof(D3D12_RESOURCE_DESC));
	vbDesrciptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vbDesrciptor.Alignment = 0;
	vbDesrciptor.Format = DXGI_FORMAT_UNKNOWN;
	vbDesrciptor.DepthOrArraySize = 1;
	vbDesrciptor.MipLevels = 1;
	vbDesrciptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	vbDesrciptor.Width = 100; //TEMP hack
	vbDesrciptor.Height = 1;
	vbDesrciptor.SampleDesc.Count = 1;
	vbDesrciptor.SampleDesc.Quality = 0;
	vbDesrciptor.Flags = D3D12_RESOURCE_FLAG_NONE;
	

	//ThrowIfFailed(
	//	m_Device->CreateCommittedResource(
	//		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
	//		D3D12_HEAP_FLAG_NONE,
	//		&vbDesrciptor,
	//		D3D12_RESOURCE_STATE_COMMON,
	//		nullptr,
	//		IID_PPV_ARGS(&m_VertBufferResource))
	//);

	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&vbDesrciptor,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_VertBufferResource))
	);


	D3D12_RESOURCE_DESC ibDesrciptor;
	ZeroMemory(&ibDesrciptor, sizeof(D3D12_RESOURCE_DESC));
	ibDesrciptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	ibDesrciptor.Format = DXGI_FORMAT_UNKNOWN;
	ibDesrciptor.DepthOrArraySize = 1;
	ibDesrciptor.MipLevels = 1;
	ibDesrciptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	ibDesrciptor.Width = 100; //TEMP hack
	ibDesrciptor.Height = 1;
	ibDesrciptor.SampleDesc.Count = 1;
	ibDesrciptor.SampleDesc.Quality = 0;

	ThrowIfFailed(
		m_Device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&ibDesrciptor,
			D3D12_RESOURCE_STATE_COMMON,
			nullptr,
			IID_PPV_ARGS(&m_IndexBufferResource))
	);

	//Create resource for output (texture)
	
	D3D12_RESOURCE_DESC outputTextDesc;
	ZeroMemory(&outputTextDesc, sizeof(D3D12_RESOURCE_DESC));
	outputTextDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	outputTextDesc.Alignment = 0;
	outputTextDesc.Width = m_Width;
	outputTextDesc.Height = m_Height;
	outputTextDesc.DepthOrArraySize = 1;
	outputTextDesc.MipLevels = 1;
	outputTextDesc.Format = m_Format;
	outputTextDesc.SampleDesc.Count = 1;
	outputTextDesc.SampleDesc.Quality = 0;
	outputTextDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	outputTextDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

	ThrowIfFailed(m_Device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&outputTextDesc,
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&m_OutputResource)));
}

void Rasterizer::BuildDesrciptorsBase()
{
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.Format = mFormat;
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	//srvDesc.Texture2D.MostDetailedMip = 0;
	//srvDesc.Texture2D.MipLevels = 1;

	//D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};

	//uavDesc.Format = mFormat;
	//uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	//uavDesc.Texture2D.MipSlice = 0;

	//resources are 

	
	//Create Desrtiptors

	//0 structured buffer vertices
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescVb = {};
	srvDescVb.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDescVb.Format = DXGI_FORMAT_UNKNOWN;
	srvDescVb.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDescVb.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

	//1 structured buffer indices
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDescIb = {};
	srvDescIb.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDescIb.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDescIb.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;

	//2 texture output
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	//m_Device->CreateShaderResourceView()


	//Create constant buffer instance
	m_ConstantBuffer = std::make_unique<BufferCreator<SceneConstantBuffer, true>>(m_Device, 1);
	D3D12_GPU_VIRTUAL_ADDRESS cbGPUAddress = m_ConstantBuffer->Resource()->GetGPUVirtualAddress();
	UINT sizeConstBuffer = CalcBufferSizeWithRounding(sizeof(SceneConstantBuffer));
	


	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbGPUAddress;
	cbvDesc.SizeInBytes = sizeConstBuffer;

	//create const buffer 
	m_Device->CreateConstantBufferView(&cbvDesc, m_ConstantsTableCPU);
	m_Device->CreateShaderResourceView(m_VertBufferResource.Get(), &srvDescVb, m_VertexBufferCPUSrv);
	m_Device->CreateShaderResourceView(m_IndexBufferResource.Get(), &srvDescIb, m_IndexBufferCPUSrv);
	m_Device->CreateUnorderedAccessView(m_OutputResource.Get(), nullptr, &uavDesc, m_OutputTextureCPUUav);
}

void Rasterizer::AssignDescriptorsStart(
	CD3DX12_CPU_DESCRIPTOR_HANDLE hCpuDescriptor,
	CD3DX12_GPU_DESCRIPTOR_HANDLE hGpuDescriptor,
	UINT descriptorSize)
{
	m_ConstantsTableCPU = hCpuDescriptor;
	m_VertexBufferCPUSrv = hCpuDescriptor.Offset(1, descriptorSize);
	m_IndexBufferCPUSrv = hCpuDescriptor.Offset(1, descriptorSize);
	m_OutputTextureCPUUav = hCpuDescriptor.Offset(1, descriptorSize);

	m_ConstantsTableGPU = hGpuDescriptor;
	m_VertexBufferGPUSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_IndexBufferGPUSrv = hGpuDescriptor.Offset(1, descriptorSize);
	m_OutputTextureGPUUav = hGpuDescriptor.Offset(1, descriptorSize);

	BuildDesrciptorsBase();
}


void Rasterizer::Execute(
	ID3D12GraphicsCommandList* cmdList,
	ID3D12RootSignature* rootSig,
	ID3D12PipelineState* computePSO,
	ID3D12Resource* inputVb,
	ID3D12Resource* inputIb,
	ID3D12Resource* constantBuffer/* = nullptr*/)
{
	cmdList->SetComputeRootSignature(rootSig);

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_VertBufferResource.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBufferResource.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));

	// Copy the input (vert buffer) to m_VertBufferResource.
	cmdList->CopyResource(m_VertBufferResource.Get(), inputVb);
	cmdList->CopyResource(m_IndexBufferResource.Get(), inputIb);

	//copy constant buffer
	if (constantBuffer != nullptr)
	{
		cmdList->CopyResource(m_ConstantBuffer->Resource(), constantBuffer);
	}

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_VertBufferResource.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	cmdList->SetPipelineState(computePSO);
	cmdList->SetComputeRootDescriptorTable(0, m_ConstantsTableGPU);
	cmdList->SetComputeRootDescriptorTable(1, m_VertexBufferGPUSrv);
	cmdList->SetComputeRootDescriptorTable(2, m_IndexBufferGPUSrv);
	cmdList->SetComputeRootDescriptorTable(3, m_OutputTextureGPUUav);

	cmdList->Dispatch(ceilf(m_Width/ 256.f), m_Height, 1);

	
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));

	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_OutputResource.Get(),
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_GENERIC_READ));
}
void Rasterizer::SetConstantBuffer(const SceneConstantBuffer& constBuffer)
{
	m_ConstantBuffer->CopyData(0, constBuffer);
}

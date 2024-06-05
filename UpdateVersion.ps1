param (
    [string]$filePath
)

function Compute-BuildNumber {
    $months = @("Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec")
    $monthDays = @(31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31)

    # Отримати поточну дату в форматі "MMM dd, yyyy"
    $currentDate = Get-Date -Format "MMM dd, yyyy"

    # Перетворення дати у відповідний формат
    $dateTime = [DateTime]::ParseExact($currentDate, "MMM dd, yyyy", $null)

    $m = 0
    $d = $dateTime.DayOfYear - 1
    $y = $dateTime.Year - 1900

    $daysFrom1900 = 365 * $y
    $daysFrom1900 += [math]::Floor(($y - 1) / 4)  # Додаємо високосні роки до поточного року

    for ($i = 0; $i -lt $m; $i++) {
        $d += $monthDays[$i]
    }

    # Додаємо 1 день за кожен високосний рік після 1900 року
    $leapYears = [math]::Floor(($y - 1 - 1900 + 1) / 4)
    $daysFrom1900 += $leapYears

    $buildNum = $d + $daysFrom1900 - 35739

    return $buildNum
}

function Format-VersionStrings {
    param (
        [int]$buildNumber
    )

    $major = 1
    $minor = [math]::Floor($buildNumber / 1000)
    $patch = $buildNumber % 1000
    $version = "$major.$minor.$patch"
    $fullVersion = "$version (Build Number $buildNumber)"

    return @{
        szVersion = $version
        c_nFullVersion = $fullVersion
		iMajor = $major
		iMinor = $minor
		iPatch = $patch
    }
}

function Update-Version {
    param (
        [string]$filePath,
        [int]$buildNumber,
        [hashtable]$versionStrings
    )

	$fileContent = Get-Content -Raw -Path $filePath

	$pattern = 'FILEVERSION\s+\d+,\d+,\d+,\d+'
	$newFileVersion = "FILEVERSION $($versionStrings.iMajor),$($versionStrings.iMinor),$($versionStrings.iPatch),$buildNumber"
	$fileContent = $fileContent -replace $pattern, $newFileVersion

	$pattern = 'PRODUCTVERSION\s+\d+,\d+,\d+,\d+'
	$newProductVersion = "PRODUCTVERSION $($versionStrings.iMajor),$($versionStrings.iMinor),$($versionStrings.iPatch),$buildNumber"
	$fileContent = $fileContent -replace $pattern, $newProductVersion

	# Оновлення інших рядків версії, якщо потрібно
	$pattern = 'VALUE "FileVersion", ".*"'
	$newFileVersionStr = 'VALUE "FileVersion", "' + $versionStrings.c_nFullVersion + '"'
	$fileContent = $fileContent -replace $pattern, $newFileVersionStr

	$pattern = 'VALUE "ProductVersion", ".*"'
	$newProductVersionStr = 'VALUE "ProductVersion", "' + $versionStrings.c_nFullVersion + '"'
	$fileContent = $fileContent -replace $pattern, $newProductVersionStr

	Set-Content -Path $filePath -Value $fileContent
}

# Обчислити номер збірки
$buildNumber = Compute-BuildNumber

# Форматувати рядки версії
$versionStrings = Format-VersionStrings -buildNumber $buildNumber

# Оновити версію у файлі .rc
Update-Version -filePath $filePath -buildNumber $buildNumber -versionStrings $versionStrings
